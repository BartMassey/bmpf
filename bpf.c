/*
 * Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

/* Bayesian Particle Filtering demo: drive a simulated
   vehicle with a simulated IMU-ish and GPS-ish device
   around and try to track it. */

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ziggurat/random.h>

typedef struct { double x, y; } ccoord;
typedef struct { double r, t; } acoord;
typedef struct { ccoord posn; acoord vel; } state;
typedef struct { state state; double weight; } particle_info;

typedef particle_info *resample(double);

static resample resample_naive, resample_optimal,
       resample_logm, resample_regular;

static const double nsecs = 100;
static const double dt = 0.1;

static int nparticles = 100;
static int sort = 0;
static resample *resampler = resample_naive;

static double avar = M_PI / 16;
static double pvar = 0.1;

static void update_state(state *p, double dt) {
    double r0 = p->vel.r + gaussian(pvar);
    double t0 = p->vel.t + gaussian(avar);
    double x0 = p->posn.x + r0 * cos(t0) * dt;
    double y0 = p->posn.y - r0 * sin(t0) * dt;

    p->vel.r = r0;
    p->vel.t = t0;
    p->posn.x = x0;
    p->posn.y = y0;
}

static state vehicle = { {0, 0}, {10, 0} };

static particle_info *particle_states[2];
static int which_particle;
static particle_info *particle;

static void init_particles(void) {
    int i;
    for (i = 0; i < 2; i++)
	particle_states[i] = malloc(nparticles * sizeof(*particle_states[0]));
    which_particle = 0;
    particle = particle_states[0];
    for (i = 0; i < nparticles; i++) {
	particle[i].state.posn.x = abs(gaussian(1));
	particle[i].state.posn.y = abs(gaussian(1));
	particle[i].state.vel.r = abs(gaussian(1));
	particle[i].state.vel.t = uniform() * M_PI_2;
    }
}

static ccoord gps_measure(void) {
    ccoord result = vehicle.posn;
    result.x += gaussian(pvar);
    result.y += gaussian(pvar);
    return result;
}

static acoord imu_measure(double dt) {
    acoord result = vehicle.vel;
    result.r += gaussian(pvar / dt);
    result.t += gaussian(avar / dt);
    return result;
}

static double gprob(double delta, double sd) {
    /* return 1.0 - erf(abs(delta) * M_SQRT1_2 / sd); ??? */
    /* return exp(-0.5 * delta * delta / (sd * sd)); ??? */
    return exp(-delta * delta * sd);
}

static double gps_prob(state *s, ccoord *gps) {
    double px = gprob(s->posn.x - gps->x, pvar);
    double py = gprob(s->posn.y - gps->y, pvar);
    return px * py;
}

static double imu_prob(state *s, acoord *imu, double dt) {
    double pr = gprob(s->vel.r - imu->r, pvar / dt);
    double pt = gprob(s->vel.t - imu->t, avar / dt);
    return pr * pt;
}

static double sum_weights(void) {
    int i;
    double t = particle[0].weight;
    for (i = 1; i < nparticles; i++)
	t += particle[i].weight;
    return t;
}

static int argmax_weight(void) {
    int i = 0, j;
    for (j = 1; j < nparticles; j++)
	if (particle[j].weight > particle[i].weight)
	    i = j;
    return i;
}

static particle_info *weighted_sample(double scale) {
    int i;
    double w = uniform() * scale;
    double t = 0;
    for (i = 0; i < nparticles; i++) {
	t += particle[i].weight;
	if (t >= w)
	    return &particle[i];
    }
#ifdef DEBUG_NAIVE
    fprintf(stderr, "total %g < target %g\n", t, w);
#endif
    abort();
}

static particle_info *resample_naive(double scale) {
    particle_info *newp = particle_states[!which_particle];
    int i;
    for (i = 0; i < nparticles; i++)
        newp[i] = *weighted_sample(scale);
    return newp;
}

static double nform(int n) {
    return 1.0 - pow(uniform(), 1.0 / (n + 1));
}

static particle_info *resample_optimal(double scale) {
    particle_info *newp = particle_states[!which_particle];
    double u0 = nform(nparticles - 1) * scale;
    int i, j = 0;
    double t = 0;
    for (i = 0; i < nparticles; i++) {
        while (t + particle[j].weight < u0 && j < nparticles)
	    t += particle[j++].weight;
#ifdef DEBUG_OPTIMAL
	if (j >= nparticles) {
	    fprintf(stderr, "fell off end s=%g t=%g u=%g\n",
		    scale, t, u0);
	    abort();
	}
#endif
	newp[i] = particle[j];
	u0 = u0 + (scale - u0) * nform(nparticles - i - 1);
    }
    return newp;
}

static particle_info *resample_regular(double scale) {
    int i, j;
    double u0, t = 0;
    particle_info *newp = particle_states[!which_particle];
    /* shuffle */
    for (i = 0; i < nparticles - 1; i++) {
	j = rand32() % (nparticles - i) + i;
	particle_info ptmp = particle[j];
	particle[j] = particle[i];
	particle[i] = ptmp;
    }
    /* merge */
    u0 = scale / (nparticles + 1);
    j = 0;
    for (i = 0; i < nparticles; i++ ) {
        while (t + particle[j].weight < u0 && j < nparticles)
	    t += particle[j++].weight;
#ifdef DEBUG_REGULAR
	if (j >= nparticles) {
	    fprintf(stderr, "fell off end s=%.14g t=%.14g u=%.14g\n",
		    scale, t, u0);
	    abort();
	}
#endif
	newp[i] = particle[j];
	u0 += scale / (nparticles + 1);
    }
    return newp;
}

static double *tweight;
#ifdef DEBUG_LOGM
static int total_depth;
#define DW 1.0e9
#endif


static particle_info *logm_weighted_sample(double scale) {
    double w = uniform() * scale;
    int i = 0;
    while(i < nparticles) {
	int left = 2 * i + 1;
	int right = 2 * i + 2;
	double lweight = 0;
	if (left < nparticles)
	    lweight = tweight[left];
#ifdef DEBUG_LOGM
	total_depth++;
#endif
	if (w < lweight) {
	    i = left;
	    continue;
	}
	if (w <= lweight + particle[i].weight)
	    return &particle[i];
	w -= lweight + particle[i].weight;
	i = right;
    }
    i = (i - 1) / 2;
#ifdef DEBUG_LOGM
    fprintf(stderr, "fell off tree on %g with i=%d, w[i]=%g\n",
	    w, i, particle[i].weight);
#endif
    abort();
}

static particle_info *resample_logm(double scale) {
    int i;
    particle_info *newp = particle_states[!which_particle];
    for (i = nparticles - 1; i >= 0; --i) {
	int left = 2 * i + 1;
	int right = 2 * i + 2;
	tweight[i] = particle[i].weight;
	if (left < nparticles)
	    tweight[i] += tweight[left];
	if (right < nparticles)
	    tweight[i] += tweight[right];
    }
#ifdef DEBUG_LOGM
    assert(tweight[0] * (1.0 - DW) <= scale &&
	   scale <= tweight[0] * (1.0 + DW));
    total_depth = 0;
#endif
    for (i = 0; i < nparticles; i++)
        newp[i] = *logm_weighted_sample(tweight[0]);
#ifdef DEBUG_LOGM
    fprintf(stderr, "%f\n", total_depth / (double) nparticles);
#endif
    return newp;
}

inline static int sgn(double x) {
    if (x < 0)
	return -1;
    if (x > 0)
	return 1;
    return 0;
}

static int cmp_weight(const void *w1, const void *w2) {
    const particle_info *p1 = w1;
    const particle_info *p2 = w2;
    return -sgn(p1->weight - p2->weight);
}

static state bpf_step(ccoord *gps, acoord *imu, double dt) {
    int i;
    particle_info *newp;
    /* update particles */
    for (i = 0; i < nparticles; i++)
	update_state(&particle[i].state, dt);
    /* do probabilistic weighting */
    for (i = 0; i < nparticles; i++) {
	double gp = gps_prob(&particle[i].state, gps);
	double ip = imu_prob(&particle[i].state, imu, dt);
        particle[i].weight = gp * ip;
    }
    /* get normalizing constant */
    double tweight = sum_weights();
    /* resample */
    if (sort)
	qsort(particle, nparticles, sizeof(particle[0]), cmp_weight);
    newp = resampler(tweight);
    /* find max weighted */
    state best = particle[argmax_weight()].state;
    /* complete */
    particle = newp;
    which_particle = !which_particle;
    return best;
}

static void run(void) {
    double t;
    init_particles();
    tweight = malloc(nparticles * sizeof(tweight[0]));
    for(t = 0; t <= nsecs; t += dt) {
	update_state(&vehicle, dt);
	printf("%g %g ", vehicle.posn.x, vehicle.posn.y);
	ccoord gps = gps_measure();
	acoord imu = imu_measure(dt);
	state est = bpf_step(&gps, &imu, dt);
	printf("%g %g\n", est.posn.x, est.posn.y);
    }
}


int main(int argc, char **argv) {
    if (argc > 1)
	nparticles = atoi(argv[1]);
    if (argc > 2) {
	if (!strcmp(argv[2], "naivesort")) {
	    sort = 1;
	    argv[2] = "naive";
	} else if (!strcmp(argv[2], "logmsort")) {
	    sort = 1;
	    argv[2] = "logm";
	}
	if (!strcmp(argv[2], "naive"))
	    resampler = resample_naive;
	else if (!strcmp(argv[2], "logm"))
	    resampler = resample_logm;
	else if (!strcmp(argv[2], "optimal"))
	    resampler = resample_optimal;
	else if (!strcmp(argv[2], "regular"))
	    resampler = resample_regular;
	else
	    abort();
    }
    run();
    return 0;
}

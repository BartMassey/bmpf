/* Copryight (C) 2007 Bart Massey
   ALL RIGHTS RESERVED */

/* Bayesian Particle Filtering demo: drive a simulated
   vehicle with a simulated IMU-ish and GPS-ish device
   around and try to track it. */

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

typedef struct { double x, y; } ccoord;
typedef struct { double r, t; } acoord;
typedef struct { ccoord posn; acoord vel; } state;
typedef struct { state state; double weight; } particle_info;

typedef particle_info *resample(double);

static resample resample_naive, resample_optimal;

static const double nsecs = 100;
static const double dt = 0.1;

int nparticles = 100;
int sort = 0;
static resample *resampler = resample_naive;

static double avar = M_PI / 16;
static double pvar = 0.1;

static double uniform(void) {
    return random() / (double) RAND_MAX;
}

static double gaussian(double sd) {
    double x1, w, y1;

    do {
	x1 = 2.0 * uniform() - 1.0;
	double x2 = 2.0 * uniform() - 1.0;
	w = x1 * x1 + x2 * x2;
    } while ( w >= 1.0 );

    w = sqrt( (-2.0 * log( w ) ) / w );
    y1 = x1 * w;
    return y1 * sd;
}

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

void init_particles(void) {
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

ccoord gps_measure(void) {
    ccoord result = vehicle.posn;
    result.x += gaussian(pvar);
    result.y += gaussian(pvar);
    return result;
}

acoord imu_measure(double dt) {
    acoord result = vehicle.vel;
    result.r += gaussian(pvar / dt);
    result.t += gaussian(avar / dt);
    return result;
}

/* XXX is this right? */
double gprob(double delta, double sd) {
    return exp(-(delta * delta * sd));
}

double gps_prob(state *s, ccoord *gps) {
    double px = gprob(s->posn.x - gps->x, pvar);
    double py = gprob(s->posn.y - gps->y, pvar);
    /* XXX is this right? */
    return px * py;
}

double imu_prob(state *s, acoord *imu) {
    double pr = gprob(s->vel.r - imu->r, pvar);
    double pt = gprob(s->vel.t - imu->t, avar);
    /* XXX is this right? */
    return pr * pt;
}

double sum_weights(void) {
    int i;
    double t = particle[0].weight;
    for (i = 1; i < nparticles; i++)
	t += particle[i].weight;
    return t;
}

int argmax_weight(void) {
    int i = 0, j;
    for (j = 1; j < nparticles; j++)
	if (particle[j].weight > particle[i].weight)
	    i = j;
    return i;
}

particle_info *weighted_sample(double scale) {
    int i;
    double w = uniform() * scale;
    double t = 0;
    for (i = 0; i < nparticles; i++) {
	t += particle[i].weight;
	if (t >= w)
	    return &particle[i];
    }
    fprintf(stderr, "total %g < target %g\n", t, w);
    abort();
}

particle_info *resample_naive(double scale) {
    particle_info *newp = particle_states[!which_particle];
    int i;
    for (i = 0; i < nparticles; i++)
        newp[i] = *weighted_sample(scale);
    return newp;
}

double nform(int n) {
    return 1.0 - pow(uniform(), 1.0 / (n + 1));
}

particle_info *resample_optimal(double scale) {
    particle_info *newp = particle_states[!which_particle];
    double u0 = nform(nparticles - 1) * scale;
    int i, j = 0;
    double t = 0;
    for (i = 0; i < nparticles; i++) {
        while (t + particle[j].weight < u0 && j < nparticles)
	    t += particle[j++].weight;
	assert(j < nparticles);
	newp[i] = particle[j];
	u0 = u0 + (scale - u0) * nform(nparticles - i - 1);
    }
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

state bpf_step(ccoord *gps, acoord *imu, double dt) {
    int i;
    particle_info *newp;
    /* update particles */
    for (i = 0; i < nparticles; i++)
	update_state(&particle[i].state, dt);
    /* do probabilistic weighting */
    for (i = 0; i < nparticles; i++) {
	double gp = gps_prob(&particle[i].state, gps);
	double ip = imu_prob(&particle[i].state, imu);
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

void run(void) {
    double t;
    init_particles();
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
	if (!strcmp(argv[2], "naive"))
	    resampler = resample_naive;
	else if (!strcmp(argv[2], "optimal"))
	    resampler = resample_optimal;
	else
	    abort();
    }
    if (argc > 3) {
	if (!strcmp(argv[3], "sort"))
	    sort = 1;
	else
	    abort();
    }
    run();
    return 0;
}

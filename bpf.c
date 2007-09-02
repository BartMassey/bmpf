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
/* XXX GCC or ISO C99 only, but in the latter case M_PI and
   friends may not exit */
extern double fmin(double, double);
extern double fmax(double, double);
#include <stdlib.h>
#include <string.h>
#include <ziggurat/random.h>
#include "exp.h"
#include "bpf.h"
#include "resample/resample.h"

static const double nsecs = 100;
static const double dt = 0.1;

static int nparticles = 100;
static int sort = 0;
static resample *resampler = resample_naive;

static double avar = M_PI / 16;
static double pvar = 0.1;

/* should be divisible by 4, and powers of 2 may
   be more efficient */
#define NDIRNS 1024
double cos_dirn[NDIRNS];

void init_dirn(void) {
    int i;
    for (i = 0; i < NDIRNS; i++) {
	double t = i * 2 * M_PI / NDIRNS;
	cos_dirn[i] = cos(t);
    }
}

static double normalize_angle(double t) {
    while (t >= 2 * M_PI)
	t -= 2 * M_PI;
    while (t < 0)
	t += 2 * M_PI;
    return t;
}

static inline int opp_dirn(int d) {
    return (d + NDIRNS / 2) % NDIRNS;
}

static inline int angle_dirn(double t) {
    return (int)(floor(t * NDIRNS)) % NDIRNS;
}

static inline int normalize_dirn(int d) {
    while (d < 0)
	d += NDIRNS;
    return d % NDIRNS;
}

static inline double clip_box(double x) {
    return fmin(10.0, fmax(x, -10.0));
}

static void update_state(state *s, double dt) {
    double r0 = fmax(s->vel.r + gaussian(pvar), 0);
    double t0 = normalize_angle(s->vel.t + gaussian(avar));
    int d0 = angle_dirn(t0);
    double x0 = clip_box(s->posn.x + r0 * cos_dirn[d0] * dt);
    double y0 = clip_box(s->posn.y + r0 *
			 cos_dirn[normalize_dirn(d0 + NDIRNS / 4)] * dt);
    s->vel.r = r0;
    s->vel.t = t0;
    s->posn.x = x0;
    s->posn.y = y0;
}

static state vehicle;

static particle_info *particle_states[2];
static int which_particle;
static particle_info *particle;

static void init_state(state *s) {
	s->posn.x = clip_box(abs(gaussian(1)));
	s->posn.y = clip_box(abs(gaussian(1)));
	s->vel.r = abs(gaussian(1));
	s->vel.t = normalize_angle(uniform() * M_PI_2);
}


static void init_particles(void) {
    int i;
    for (i = 0; i < 2; i++)
	particle_states[i] = malloc(nparticles * sizeof(*particle_states[0]));
    which_particle = 0;
    particle = particle_states[0];
    for (i = 0; i < nparticles; i++)
	init_state(&particle[i].state);
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
    result.t = normalize_angle(result.t + gaussian(avar / dt));
    if (result.r < 0) {
	result.r = - result.r;
	result.t = normalize_angle(result.t + M_PI);
    }
    return result;
}

static double gprob(double delta, double sd) {
    /* return 1.0 - erf(abs(delta) * M_SQRT1_2 / sd); ??? */
    /* return exp(-0.5 * delta * delta / (sd * sd)); ??? */
    return exp_(-delta * delta * sd);
}

static double gps_prob(state *s, ccoord *gps) {
    if (s->posn.x != clip_box(s->posn.x) ||
	s->posn.y != clip_box(s->posn.y))
	return 0;
    double px = gprob(s->posn.x - gps->x, pvar);
    double py = gprob(s->posn.y - gps->y, pvar);
    return px * py;
}

static double imu_prob(state *s, acoord *imu, double dt) {
    double pr = gprob(s->vel.r - imu->r, pvar / dt);
    double dth = fmin(fabs(s->vel.t - imu->t),
		      fabs(fabs(s->vel.t - imu->t) - 2 * M_PI));
    double pt = gprob(dth, avar / dt);
    return pr * pt;
}

static state bpf_step(ccoord *gps, acoord *imu, double dt) {
    int i;
    particle_info *newp;
    double tweight = 0;
    int best;
    /* update particles */
    for (i = 0; i < nparticles; i++) {
	double w;
	update_state(&particle[i].state, dt);
	/* do probabilistic weighting */
	double gp = gps_prob(&particle[i].state, gps);
	double ip = imu_prob(&particle[i].state, imu, dt);
	w = gp * ip;
        particle[i].weight = w;
	tweight += w;
    }
    /* resample */
    newp = particle_states[!which_particle];
    best = resampler(tweight, nparticles, particle, nparticles, newp, sort);
    /* complete */
    particle = newp;
    which_particle = !which_particle;
    return particle[best].state;
}

static void run(void) {
    double t;
    init_particles();
    init_state(&vehicle);
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
    struct resample_info *entry;
    if (argc > 1)
	nparticles = atoi(argv[1]);
    if (argc > 2) {
	int na = strlen(argv[2]);
	int ns = strlen("sort");
	if (na > ns) {
	    if(!strcmp(argv[2] + na - ns, "sort")) {
		sort = 1;
		argv[2][na - ns] = '\0';
	    }
	}
	for (entry = resamplers; entry->name; entry++) {
	    if (!strcmp(argv[2], entry->name)) {
		if (entry->f_init)
		    entry->f_init(nparticles, nparticles);
		resampler = entry->f;
		break;
	    }
	}
    }
    assert(resampler);
    init_dirn();
    run();
    return 0;
}

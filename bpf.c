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
#include "exp.h"

typedef struct { double x, y; } ccoord;
typedef struct { double r, t; } acoord;
typedef struct { ccoord posn; acoord vel; } state;
typedef struct { state state; double weight; } particle_info;

/* returns a highest-weighted particle index */
typedef int resample(double, particle_info *, particle_info *);
/* accepts a maximum number of particles input / output */
typedef void init_resample(int, int);

static init_resample init_resample_logm;
static resample resample_naive, resample_optimal,
       resample_logm, resample_regular;

static const double nsecs = 100;
static const double dt = 0.1;

static int nparticles = 100;
static int sort = 0;
static resample *resampler = resample_naive;

static double avar = M_PI / 16;
static double pvar = 0.1;

#define NDIRNS 256
typedef struct {
    double ux;
    double uy;
} unit_vector;
unit_vector dirn[NDIRNS];

void init_dirn(void) {
    int i;
    for (i = 0; i < NDIRNS; i++) {
	double t = i * 2 * M_PI / NDIRNS;
	dirn[i].ux = cos(t);
	dirn[i].uy = -sin(t);
    }
}

static void update_state(state *p, double dt) {
    double r0 = p->vel.r + gaussian(pvar);
    double t0 = p->vel.t + gaussian(avar);
    int d0 = (int)(floor(t0 * NDIRNS)) % NDIRNS;
    double x0 = p->posn.x + r0 * dirn[d0].ux * dt;
    double y0 = p->posn.y - r0 * dirn[d0].uy * dt;
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
    return exp_(-delta * delta * sd);
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
    best = resampler(tweight, particle, newp);
    /* complete */
    particle = newp;
    which_particle = !which_particle;
    return particle[best].state;
}

static void run(void) {
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

#include "resample/naive.h"
#include "resample/optimal.h"
#include "resample/logm.h"
#include "resample/regular.h"

static struct resample_info {
    char *name;
    init_resample *f_init;
    resample *f;
} resamplers[] = {
    {"naive", 0, resample_naive},
    {"logm", init_resample_logm, resample_logm},
    {"optimal", 0, resample_optimal},
    {"regular", 0, resample_regular},
    {0, 0}
};

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

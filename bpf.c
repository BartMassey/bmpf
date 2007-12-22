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
   friends may not exist */
extern double fmin(double, double);
extern double fmax(double, double);
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE
#include <getopt.h>
#undef _GNU_SOURCE
#include "bpf.h"
#include "sim.h"
#include "resample/resample.h"


static int report_particles, best_particle;
static int resample_interval = 1;

static struct option options[] = {
    {"report-particles", 0, 0, 'r'},
    {"best-particle", 0, 0, 'b'},
    {"fast-direction", 0, 0, 'd'},
    {"resample-interval", 1, 0, 's'},
    {"angle-variance", 1, 0, 'A'},
    {"velocity-variance", 1, 0, 'V'},
    {"gps-variance", 1, 0, 'G'},
    {"imu-angle-variance", 1, 0, 'R'},
    {"imu-velocity-variance", 1, 0, 'T'},
    {0, 0, 0, 0}
};

static const double nsecs = 100;
static const double dt = 0.1;

static int nparticles = 100;
static int sort = 0;
static resample *resampler = resample_naive;

static particle_info *particle_states[2];
static int which_particle;
static particle_info *particle;

static state vehicle;

static void init_particles(void) {
    double invscale = 1.0 / nparticles;
    int i;
    for (i = 0; i < 2; i++)
	particle_states[i] = malloc(nparticles * sizeof(*particle_states[0]));
    which_particle = 0;
    particle = particle_states[0];
    for (i = 0; i < nparticles; i++) {
	init_state(&particle[i].state);
	particle[i].weight = invscale;
    }
}

static ccoord gps_measure(void) {
    ccoord result = vehicle.posn;
    result.x += gaussian(gps_var);
    result.y += gaussian(gps_var);
    return result;
}

static acoord imu_measure(double dt) {
    acoord result = vehicle.vel;
    result.r += gaussian(imu_r_var * dt);
    result.t = normalize_angle(result.t + gaussian(imu_a_var * dt));
    if (result.r < 0) {
	result.r = - result.r;
	result.t = normalize_angle(result.t + M_PI);
    }
    return result;
}

static double gprob(double delta, double sd) {
    return erfc(fabs(delta) * M_SQRT1_2 / sd);
}

static double gps_prob(state *s, ccoord *gps) {
    if (s->posn.x != clip_box(s->posn.x) ||
	s->posn.y != clip_box(s->posn.y))
	return 0;
    double px = gprob(s->posn.x - gps->x, gps_var);
    double py = gprob(s->posn.y - gps->y, gps_var);
    return px * py;
}

static double imu_prob(state *s, acoord *imu, double dt) {
    if (s->vel.r != clip_speed(s->vel.r))
	return 0;
    double pr = gprob(s->vel.r - imu->r, imu_r_var / dt);
    double dth = fmin(fabs(s->vel.t - imu->t),
		      fabs(fabs(s->vel.t - imu->t) - 2 * M_PI));
    double pt = gprob(dth, imu_a_var / dt);
    return pr * pt;
}

void bpf_step(ccoord *gps, acoord *imu,
	      double t, double dt, int report) {
    int i;
    particle_info *newp;
    double tweight = 0;
    double invtweight;
    int best;
    state est_state;
    static int resample_count = 0;
    /* update particles */
    for (i = 0; i < nparticles; i++) {
	double w;
	update_state(&particle[i].state, dt);
	/* do probabilistic weighting */
	double gp = gps_prob(&particle[i].state, gps);
	double ip = imu_prob(&particle[i].state, imu, dt);
        w = particle[i].weight * gp * ip;
	particle[i].weight = w;
	tweight += w;
    }
    est_state.posn.x = 0;
    est_state.posn.y = 0;
    est_state.vel.r = 0;
    est_state.vel.t = 0;
    if (!best_particle) {
	invtweight = 1.0 / tweight;
	for (i = 0; i < nparticles; i++) {
	    state *s = &particle[i].state;
	    double w = particle[i].weight * invtweight;
	    est_state.posn.x += w * s->posn.x;
	    est_state.posn.y += w * s->posn.y;
	    est_state.vel.r += w * s->vel.r;
	    est_state.vel.t = normalize_angle(est_state.vel.t + w * s->vel.t);
	}
    }
    if (report) {
	double vx = vehicle.posn.x;
	double vy = vehicle.posn.y;
	FILE *fp;
	char fn[128];
	sprintf(fn, "benchtmp/particles-%g.dat", t);
	fp = fopen(fn, "w");
	assert(fp);
	for (i = 0; i < nparticles; i++) {
	    double px = particle[i].state.posn.x;
	    double py = particle[i].state.posn.y;
	    fprintf(fp, "%g %g\n", px - vx, py - vy);
	}
	fclose(fp);
    }
    resample_count = (resample_count + 1) % resample_interval;
    if (resample_count == 0) {
	/* resample */
	newp = particle_states[!which_particle];
	best = resampler(tweight, nparticles, particle, nparticles, newp, sort);
	/* complete */
	particle = newp;
	which_particle = !which_particle;
    } else {
	double best_weight = particle[0].weight;
	best = 0;
	for (i = 1; i < nparticles; i++) {
	    if (particle[i].weight > best_weight) {
		best = i;
		best_weight = particle[i].weight;
	    }
	}
    }
    printf(" %g %g",
	   particle[best].state.posn.x,
	   particle[best].state.posn.y);
    if (!best_particle) {
	printf(" %g %g",
	       est_state.posn.x,
	       est_state.posn.y);
    }
}

static void run(void) {
    double t;
    init_particles();
    init_state(&vehicle);
    for(t = 0; t <= nsecs; t += dt) {
	int msecs = floor(t * 1000 + 0.5);
	int report = report_particles && !(msecs % 10000);
	update_state(&vehicle, dt);
	printf("%g %g", vehicle.posn.x, vehicle.posn.y);
	ccoord gps = gps_measure();
	acoord imu = imu_measure(dt);
	bpf_step(&gps, &imu, t, dt, report);
	printf("\n");
    }
}

int main(int argc, char **argv) {
    struct resample_info *entry;
    while (1) {
	int c = getopt_long(argc, argv, "rbds:", options, &optind);
	if (c == -1)
	    break;
	switch (c) {
	case 'r':
	    report_particles = 1;
	    break;
	case 'b':
	    best_particle = 1;
	    break;
	case 'd':
	    fast_direction = 1;
	    break;
	case 'A':
	    avar = atof(optarg);
	    break;
        case 'V':
	    rvar = atof(optarg);
	    break;
	case 'G':
	    gps_var = atof(optarg);
	    break;
	case 'R':
	    imu_r_var = atof(optarg);
	    break;
	case 'T':
	    imu_a_var = atof(optarg);
	    break;
	case 's':
	    resample_interval = atoi(optarg);
	    break;
	default:
	    fprintf(stderr, "bpf: usage error\n");
	    exit(1);
	}
    }
    if (optind < argc)
	nparticles = atoi(argv[optind++]);
    if (optind < argc) {
	int na = strlen(argv[optind]);
	int ns = strlen("sort");
	if (na > ns) {
	    if(!strcmp(argv[optind] + na - ns, "sort")) {
		sort = 1;
		argv[optind][na - ns] = '\0';
	    }
	}
	for (entry = resamplers; entry->name; entry++) {
	    if (!strcmp(argv[optind], entry->name)) {
		if (entry->f_init)
		    entry->f_init(nparticles, nparticles);
		resampler = entry->f;
		break;
	    }
	}
    }
    assert(resampler);
    if (fast_direction)
	init_dirn();
    run();
    return 0;
}

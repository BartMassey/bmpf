/*
 * Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

/* Bayesian Particle Filtering demo: Try to track a
   simulated vehicle with a simulated IMU-ish and GPS-ish
   device. */

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
#include "exp.h"

static int report_particles, best_particle;
static int resample_interval = 1;

static struct option options[] = {
    {"report-particles", 1, 0, 'r'},
    {"best-particle", 0, 0, 'b'},
    {"resample-interval", 1, 0, 's'},
    {"fast-direction", 0, 0, 'd'},
    {"angle-variance", 1, 0, 'A'},
    {"velocity-variance", 1, 0, 'V'},
    {"gps-variance", 1, 0, 'G'},
    {"imu-angle-variance", 1, 0, 'R'},
    {"imu-velocity-variance", 1, 0, 'T'},
    {0, 0, 0, 0}
};

static int nparticles = 100;
static int sort = 0;
static resample *resampler = resample_naive;

static particle_info *particle_states[2];
static int which_particle;
static particle_info *particle;

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

static double gprob(double delta, double sd) {
    return exp(-0.5 * delta * delta / (sd * sd));
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

void bpf_step(ccoord *vehicle, ccoord *gps, acoord *imu,
	      double t, double dt, int report) {
    int i;
    particle_info *newp;
    double tweight;
    double invtweight;
    int best;
#ifdef DIAGNOSTIC_PRINT
    int worst;
#endif
    double best_weight, worst_weight;
    state est_state;
    static int resample_count = 0;
#ifdef DEBUG
    tweight = 0;
    for (i = 0; i < nparticles; i++)
	tweight += particle[i].weight;
    assert(tweight > 0.00001);
#endif
    /* update particles */
    tweight = 0;
    for (i = 0; i < nparticles; i++) {
	double w;
	update_state(&particle[i].state, dt, 1);
	/* do probabilistic weighting */
	double gp = gps_prob(&particle[i].state, gps);
	double ip = imu_prob(&particle[i].state, imu, dt);
        w = gp * ip * particle[i].weight;
	particle[i].weight = w;
	tweight += w;
    }
#ifdef DEBUG
    assert(tweight > 0.00001);
#endif
    invtweight = 1.0 / tweight;
    for (i = 0; i < nparticles; i++)
	particle[i].weight = particle[i].weight * invtweight;
    est_state.posn.x = 0;
    est_state.posn.y = 0;
    est_state.vel.r = 0;
    est_state.vel.t = 0;
    if (!best_particle) {
	for (i = 0; i < nparticles; i++) {
	    state *s = &particle[i].state;
	    double w = particle[i].weight;
	    est_state.posn.x += w * s->posn.x;
	    est_state.posn.y += w * s->posn.y;
	    est_state.vel.r += w * s->vel.r;
	    est_state.vel.t = normalize_angle(est_state.vel.t + w * s->vel.t);
	}
    }
    if (report) {
	FILE *fp;
	char fn[128];
	sprintf(fn, "benchtmp/particles-%g.dat", t);
	fp = fopen(fn, "w");
	assert(fp);
	for (i = 0; i < nparticles; i++) {
	    double px = particle[i].state.posn.x;
	    double py = particle[i].state.posn.y;
            double w = particle[i].weight;
	    fprintf(fp, "%g %g %g\n", px, py, w);
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
	for (i = 0; i < nparticles; i++)
	    particle[i].weight = 1.0 / nparticles;
    }
    {
	best_weight = particle[0].weight;
	worst_weight = particle[0].weight;
	best = 0;
#ifdef DIAGNOSTIC_PRINT
	worst = 0;
#endif
	for (i = 1; i < nparticles; i++) {
	    if (particle[i].weight > best_weight) {
		best = i;
		best_weight = particle[i].weight;
	    } else if (particle[i].weight < worst_weight) {
#ifdef DIAGNOSTIC_PRINT
		worst = i;
#endif
		worst_weight = particle[i].weight;
	    }
	}
    }
#ifdef DIAGNOSTIC_PRINT
    printf("  %g %g %g",
	   best_weight,
	   particle[best].state.posn.x,
	   particle[best].state.posn.y);
    printf("  %g %g %g",
	   worst_weight,
	   particle[worst].state.posn.x,
	   particle[worst].state.posn.y);
#else
    printf("  %g %g",
	   particle[best].state.posn.x,
	   particle[best].state.posn.y);
#endif
    if (!best_particle) {
	printf("  %g %g",
	       est_state.posn.x,
	       est_state.posn.y);
    }
}

static int read_inputs(ccoord *vehicle, ccoord *gps, acoord *imu, int *t_ms) {
    int n = scanf("%d %lg %lg %lg %lg %lg %lg\n", t_ms,
		  &vehicle->x, &vehicle->y,
		  &gps->x, &gps->y, &imu->r, &imu->t);
    if (n == 7)
	return 1;
    if (ferror(stdin)) {
	perror("bpf");
	exit(1);
    }
    assert(n == EOF);
    return 0;
}

static void run(void) {
    ccoord vehicle, gps;
    acoord imu;
    double t;
    int t_ms;
    int t_last = 0;
    int i;
    init_particles();
    for (i = 0; i < nparticles; i++)
	particle[i].weight = 1.0 / nparticles;
    /* XXX for now, we dont want to deal with dt==0 */
    read_inputs(&vehicle, &gps, &imu, &t_ms);
    t = t_ms * (1.0 / 1000.0);
    while (read_inputs(&vehicle, &gps, &imu, &t_ms)) {
	double t0 = t_ms * (1.0 / 1000.0);
	double dt = t0 - t;
        int report = 0;
	if (report_particles > 0)
            report = t_ms - t_last >= report_particles;
	t = t0;
	printf("%g %g", vehicle.x, vehicle.y);
	bpf_step(&vehicle, &gps, &imu, t, dt, report);
        if (report)
	    t_last = t_ms;
	printf("\n");
    }
}

int main(int argc, char **argv) {
    struct resample_info *entry;
    while (1) {
	int c = getopt_long(argc, argv, "r:bs:dA:V:G:R:T:", options, &optind);
	if (c == -1)
	    break;
	switch (c) {
	case 'r':
	    report_particles = atoi(optarg);
	    break;
	case 'b':
	    best_particle = 1;
	    break;
	case 's':
	    resample_interval = atoi(optarg);
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
	default:
	    fprintf(stderr, "bpf: usage error\n");
	    exit(1);
	}
    }
    if (optind < argc)
	nparticles = atoi(argv[optind++]);
    assert(nparticles > 0);
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
        assert(entry->name);
        optind++;
    }
    assert(optind == argc);
    if (fast_direction)
	init_dirn();
    run();
    return 0;
}

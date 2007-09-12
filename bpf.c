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

static double avar = M_PI / 32;
static double rvar = 0.1;
static double gps_var = 5.0;
static double imu_r_var = 0.5;
static double imu_a_var = M_PI / 8;
#define BOX_DIM 20.0
#define MAX_SPEED 2.0

#ifdef FAST_DIRN

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

static inline int angle_dirn(double t) {
    return (int)(floor(t * NDIRNS / (2 * M_PI))) % NDIRNS;
}

static inline int normalize_dirn(int d) {
    while (d < 0)
	d += NDIRNS;
    return d % NDIRNS;
}

#endif

static double normalize_angle(double t) {
    while (t >= 2 * M_PI)
	t -= 2 * M_PI;
    while (t < 0)
	t += 2 * M_PI;
    return t;
}

static inline double clip(double x, double low, double high) {
    return fmin(high, fmax(x, low));
}

static inline double clip_box(double x) {
    return clip(x, -BOX_DIM, BOX_DIM);
}

static inline double clip_speed(double x) {
    return clip(x, 0, MAX_SPEED);
}

enum bounce_problem {
    BOUNCE_OK,
    BOUNCE_X,
    BOUNCE_Y,
    BOUNCE_XY
};

static enum bounce_problem bounce(double r, double t, state *s, double dt) {
    double x0, y0, x1, y1;
#ifdef FAST_DIRN
    int dc0, dms0;
    dc0 = angle_dirn(t);
    dms0 = normalize_dirn(dc0 + NDIRNS / 4);
    x0 = s->posn.x + r * cos_dirn[dc0] * dt;
    y0 = s->posn.y + r * cos_dirn[dms0] * dt;
#else
    x0 = s->posn.x + r * cos(t) * dt;
    y0 = s->posn.y - r * sin(t) * dt;
#endif
    x1 = clip_box(x0);
    y1 = clip_box(y0);
    if (x0 == x1 && y0 == y1) {
	s->posn.x = x1;
	s->posn.y = y1;
	s->vel.t = t;
	s->vel.r = r;
	return BOUNCE_OK;
    }
#ifdef FAST_DIRN
    x0 = s->posn.x + r * cos(t) * dt;
    y0 = s->posn.y - r * sin(t) * dt;
    x1 = clip_box(x0);
    y1 = clip_box(y0);
    if (x0 == x1 && y0 == y1) {
	s->posn.x = x1;
	s->posn.y = y1;
	s->vel.t = t;
	s->vel.r = r;
	return BOUNCE_OK;
    }
#endif    
    if (y0 == y1)
	return BOUNCE_X;
    if (x0 == x1)
	return BOUNCE_Y;
    return BOUNCE_XY;
}

static void update_state(state *s, double dt) {
    double r0 = clip_speed(s->vel.r + gaussian(rvar));
    double t0 = normalize_angle(s->vel.t + gaussian(avar));
    enum bounce_problem b = bounce(r0, t0, s, dt);
    if (b != BOUNCE_OK) {
	r0 = s->vel.r;
	t0 = s->vel.t;
	b = bounce(r0, t0, s, dt);
	switch (b) {
	case BOUNCE_X:
	    t0 = normalize_angle(M_PI - t0);
	    b = bounce(r0, t0, s, dt);
	    break;
	case BOUNCE_Y:
	    t0 = normalize_angle(2 * M_PI - t0);
	    b = bounce(r0, t0, s, dt);
	    break;
	case BOUNCE_XY:
	    t0 = normalize_angle(M_PI + t0);
	    b = bounce(r0, t0, s, dt);
	    break;
	case BOUNCE_OK:
	    break;
	}
    }
    assert(b == BOUNCE_OK);
}

static state vehicle;

static particle_info *particle_states[2];
static int which_particle;
static particle_info *particle;

static void init_state(state *s) {
	s->posn.x = clip_box(fabs(gaussian(1.0)));
	s->posn.y = clip_box(fabs(gaussian(1.0)));
	s->vel.r = fabs(gaussian(1.0));
	s->vel.t = normalize_angle(uniform() * M_PI_2);
}


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
    return 1.0 - erf(fabs(delta) * M_SQRT1_2 / sd);
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
#ifndef FAST_EST
    state est_state;
#endif
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
#ifndef FAST_EST
    est_state.posn.x = 0;
    est_state.posn.y = 0;
    est_state.vel.r = 0;
    est_state.vel.t = 0;
    invtweight = 1.0 / tweight;
    for (i = 0; i < nparticles; i++) {
	state *s = &particle[i].state;
	double w = particle[i].weight * invtweight;
	est_state.posn.x += w * s->posn.x;
	est_state.posn.y += w * s->posn.y;
	est_state.vel.r += w * s->vel.r;
	est_state.vel.t = normalize_angle(est_state.vel.t + w * s->vel.t);
    }
#endif
#ifdef REPORT
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
#endif
    /* resample */
    newp = particle_states[!which_particle];
    best = resampler(tweight, nparticles, particle, nparticles, newp, sort);
    /* complete */
    particle = newp;
    which_particle = !which_particle;
    printf(" %g %g",
	   particle[best].state.posn.x,
	   particle[best].state.posn.y);
#ifndef FAST_EST
    printf(" %g %g",
	   est_state.posn.x,
	   est_state.posn.y);
#endif
}

static void run(void) {
    double t;
    init_particles();
    init_state(&vehicle);
    for(t = 0; t <= nsecs; t += dt) {
	int msecs = floor(t * 1000 + 0.5);
	int report = !(msecs % 10000);
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
#ifdef FAST_DIRN
    init_dirn();
#endif
    run();
    return 0;
}

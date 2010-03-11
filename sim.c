/*
 * Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

/* Vehicle simulator code. */

#include <assert.h>
#include <stdio.h>
#include <math.h>
/* XXX GCC or ISO C99 only, but in the latter case M_PI and
   friends may not exist */
extern double fmin(double, double);
extern double fmax(double, double);
#include <stdlib.h>
#include <string.h>
#include <zrandom.h>
#include "bpf.h"
#include "exp.h"
#include "sim.h"

double avar = M_PI / 32;
double rvar = 0.1;
double gps_var = 1.0;
double imu_r_var = 0.5;
double imu_a_var = M_PI / 8;

int fast_direction = 0;
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

enum bounce_problem {
    BOUNCE_OK,
    BOUNCE_X,
    BOUNCE_Y,
    BOUNCE_XY
};

static enum bounce_problem bounce(double r, double t, state *s,
				  double dt, int noise) {
    double x0, y0, x1, y1;
    int dc0, dms0;
    if (fast_direction) {
	dc0 = angle_dirn(t);
	dms0 = normalize_dirn(dc0 + NDIRNS / 4);
	x0 = s->posn.x + r * cos_dirn[dc0] * dt;
	y0 = s->posn.y + r * cos_dirn[dms0] * dt;
    } else {
	x0 = s->posn.x + r * cos(t) * dt;
	y0 = s->posn.y - r * sin(t) * dt;
    }
    x1 = clip_box(x0);
    y1 = clip_box(y0);
    if (x0 == x1 && y0 == y1) {
	s->posn.x = x1;
	s->posn.y = y1;
	s->vel.t = t;
	s->vel.r = r;
	return BOUNCE_OK;
    }
    if (fast_direction) {
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
    }
    if (y0 == y1)
	return BOUNCE_X;
    if (x0 == x1)
	return BOUNCE_Y;
    return BOUNCE_XY;
}

void init_state(state *s) {
	s->posn.x = clip_box(fabs(gaussian(1.0)));
	s->posn.y = clip_box(fabs(gaussian(1.0)));
	s->vel.r = fabs(gaussian(1.0));
	s->vel.t = normalize_angle(uniform() * M_PI_2);
}


void update_state(state *s, double dt, int noise) {
    double r0 = clip_speed(s->vel.r + gaussian(rvar) * (1 + 8 * noise));
    double t0 = normalize_angle(s->vel.t + gaussian(avar) * (1 +  8 * noise));
    enum bounce_problem b = bounce(r0, t0, s, dt, noise);
    if (b != BOUNCE_OK) {
	r0 = s->vel.r;
	t0 = s->vel.t;
	b = bounce(r0, t0, s, dt, 0);
	switch (b) {
	case BOUNCE_X:
	    t0 = normalize_angle(M_PI - t0);
	    b = bounce(r0, t0, s, dt, 0);
	    break;
	case BOUNCE_Y:
	    t0 = normalize_angle(2 * M_PI - t0);
	    b = bounce(r0, t0, s, dt, 0);
	    break;
	case BOUNCE_XY:
	    t0 = normalize_angle(M_PI + t0);
	    b = bounce(r0, t0, s, dt, 0);
	    break;
	case BOUNCE_OK:
	    break;
	}
    }
    assert(b == BOUNCE_OK);
}


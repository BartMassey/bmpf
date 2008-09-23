/*
 * Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

/* Vehicle simulator for Bayesian Particle Filtering demo:
   includes a simulated IMU-ish and GPS-ish device. */

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
#include <ziggurat/random.h>
#include "bpf.h"
#include "sim.h"

static struct option options[] = {
    {"fast-direction", 0, 0, 'd'},
    {"angle-variance", 1, 0, 'A'},
    {"velocity-variance", 1, 0, 'V'},
    {"gps-variance", 1, 0, 'G'},
    {"imu-angle-variance", 1, 0, 'R'},
    {"imu-velocity-variance", 1, 0, 'T'},
    {0, 0, 0, 0}
};

static state vehicle;

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

void run(void) {
    double t, dt = 0.01;

    init_state(&vehicle);
    for (t = 0; t <= 10.0; t += dt) {
	int msecs = floor(t * 1000 + 0.5);
	update_state(&vehicle, dt, 0);
	ccoord gps = gps_measure();
	acoord imu = imu_measure(dt);
	printf("%d %g %g %g %g %g %g\n", msecs,
	       vehicle.posn.x, vehicle.posn.y,
	       gps.x, gps.y, imu.r, imu.t);
    }
}

int main(int argc, char **argv) {
    while (1) {
	int c = getopt_long(argc, argv, "dA:V:G:R:T:", options, &optind);
	if (c == -1)
	    break;
	switch (c) {
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
    run();
    return 0;
}

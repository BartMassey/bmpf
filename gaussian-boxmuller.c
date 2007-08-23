/*
 * Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

/* Gaussian PRNG based on Box-Muller transform,
   with saved extra variable to cut costs. */

#include "uniform.h"

static double gaussian(double sd) {
    double x1, x2, w, y1;
    static double y2;
    static int have_y2 = 0;

    if (have_y2) {
	have_y2 = 0;
	return y2 * sd;
    }
    do {
	x1 = 2.0 * uniform() - 1.0;
	x2 = 2.0 * uniform() - 1.0;
	w = x1 * x1 + x2 * x2;
    } while ( w >= 1.0 );

    w = sqrt( (-2.0 * log( w ) ) / w );
    y1 = x1 * w;
    y2 = x2 * w;
    have_y2 = 1;
    return y1 * sd;
}

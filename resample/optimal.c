/*
 * Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

#include "resample.h"

static double nform(int n) {
    return 1.0 - pow(uniform(), 1.0 / (n + 1));
}

int resample_optimal(double scale,
		     int m, particle_info *particle,
		     int n, particle_info *newp,
		     int sort) {
    double u0 = nform(n - 1) * scale;
    int i, j = 0;
    double t = 0;
    double best_w = 0;
    int best_i = 0;
    for (i = 0; i < n; i++) {
        while (t + particle[j].weight < u0 && j < m)
	    t += particle[j++].weight;
#ifdef DEBUG_OPTIMAL
	if (j >= m) {
	    fprintf(stderr, "fell off end s=%g t=%g u=%g\n",
		    scale, t, u0);
	    abort();
	}
#endif
	newp[i] = particle[j];
        if (newp[i].weight > best_w) {
	    best_w = newp[i].weight;
	    best_i = i;
	}
	u0 = u0 + (scale - u0) * nform(n - i - 1);
    }
    return best_i;
}

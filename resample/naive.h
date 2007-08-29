/*
 * Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

static particle_info *weighted_sample(double scale) {
    int i;
    double w = uniform() * scale;
    double t = 0;
    for (i = 0; i < nparticles; i++) {
	t += particle[i].weight;
	if (t >= w)
	    return &particle[i];
    }
#ifdef DEBUG_NAIVE
    fprintf(stderr, "total %g < target %g\n", t, w);
#endif
    abort();
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

int resample_naive(double scale,
		   particle_info *particle,
		   particle_info *newp) {
    int i;
    double best_w = 0;
    int best_i = 0;
    if (sort)
	qsort(particle, nparticles, sizeof(particle[0]), cmp_weight);
    for (i = 0; i < nparticles; i++) {
        newp[i] = *weighted_sample(scale);
        if (newp[i].weight > best_w) {
	    best_w = newp[i].weight;
	    best_i = i;
	}
    }
    return best_i;
}

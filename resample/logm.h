/*
 * Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

static double *tweight;
#ifdef DEBUG_LOGM
static int total_depth;
#define DW 1.0e9
#endif


static particle_info *logm_weighted_sample(double scale) {
    double w = uniform() * scale;
    int i = 0;
    while(i < nparticles) {
	int left = 2 * i + 1;
	int right = 2 * i + 2;
	double lweight = 0;
	if (left < nparticles)
	    lweight = tweight[left];
#ifdef DEBUG_LOGM
	total_depth++;
#endif
	if (w < lweight) {
	    i = left;
	    continue;
	}
	if (w <= lweight + particle[i].weight)
	    return &particle[i];
	w -= lweight + particle[i].weight;
	i = right;
    }
    i = (i - 1) / 2;
#ifdef DEBUG_LOGM
    fprintf(stderr, "fell off tree on %g with i=%d, w[i]=%g\n",
	    w, i, particle[i].weight);
#endif
    abort();
}

static void heapify(void) {
    int i;
    for (i = nparticles / 2 - 1; i >= 0; --i) {
	int j = i;
	while (j < nparticles / 2) {
	    int left = 2 * j + 1;
	    int right = 2 * j + 2;
	    particle_info ptmp = particle[j];
	    double wj = particle[j].weight;
	    double wleft = particle[left].weight;
	    int nextj = left;
	    if (right < nparticles) {
		double wright = particle[right].weight;
		if (wj >= wleft && wj >= wright)
		    break;
		if (wj < wright && (wj >= wleft || wright > wleft))
		    nextj = right;
	    } else {
		if (wj >= wleft)
		    break;
	    }
	    particle[j] = particle[nextj];
	    particle[nextj] = ptmp;
	    j = nextj;
	}
    }
}

int resample_logm(double scale,
		  particle_info *particle,
		  particle_info *newp) {
    int i;
    double best_w = 0;
    int best_i = 0;
    if (sort) {
	heapify();
#ifdef DEBUG_HEAPIFY
	for (i = nparticles - 1; i > 0; --i)
	    assert (particle[i].weight <= particle[(i - 1) / 2].weight);
#endif
    }
    for (i = nparticles - 1; i >= 0; --i) {
	int left = 2 * i + 1;
	int right = 2 * i + 2;
	tweight[i] = particle[i].weight;
	if (left < nparticles)
	    tweight[i] += tweight[left];
	if (right < nparticles)
	    tweight[i] += tweight[right];
    }
#ifdef DEBUG_LOGM
    assert(tweight[0] * (1.0 - DW) <= scale &&
	   scale <= tweight[0] * (1.0 + DW));
    total_depth = 0;
#endif
    for (i = 0; i < nparticles; i++) {
        newp[i] = *logm_weighted_sample(tweight[0]);
        if (newp[i].weight > best_w) {
	    best_w = newp[i].weight;
	    best_i = i;
	}
    }
#ifdef DEBUG_LOGM
    fprintf(stderr, "%f\n", total_depth / (double) nparticles);
#endif
    return best_i;
}


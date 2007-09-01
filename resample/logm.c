/*
 * Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

#include <stdlib.h>
#include "resample.h"

#ifdef DEBUG_LOGM
static int total_depth;
#define DEBUG_HEAPIFY
#endif
#ifdef DEBUG_HEAPIFY
#define DW 1.0e9
#endif


static double *tweight;

extern void init_resample_logm(int mmax, int nmax) {
    tweight = malloc(mmax * sizeof(tweight[0]));
}

static particle_info *logm_weighted_sample(int m,
					   particle_info *particle,
					   double scale) {
    double w = uniform() * scale;
    int i = 0;
    while(i < m) {
	int left = 2 * i + 1;
	int right = 2 * i + 2;
	double lweight = 0;
	if (left < m)
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

static void heapify(int m, particle_info *particle) {
    int i;
    for (i = m - 1; i >= 0; --i) {
	int left = 2 * i + 1;
	int right = 2 * i + 2;
	int j = i;
	tweight[i] = particle[i].weight;
	if (i >= m / 2)
	    continue;
	tweight[i] += tweight[left];
	if (right < m)
	    tweight[i] += tweight[right];
	while (j < m / 2) {
	    int left = 2 * j + 1;
	    int right = 2 * j + 2;
	    particle_info ptmp = particle[j];
	    double wj = particle[j].weight;
	    double wleft = particle[left].weight;
	    double dw;
	    int nextj = left;
	    if (right < m) {
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
	    dw = particle[j].weight - particle[nextj].weight;
#ifdef DEBUG_LOGM
	    assert(dw >= 0);
#endif
	    tweight[nextj] -= dw;
	    j = nextj;
	}
    }
}

void init_tweights(int m, particle_info *particle) {
    int i;
    for (i = m - 1; i >= 0; --i) {
	int left = 2 * i + 1;
	int right = 2 * i + 2;
	tweight[i] = particle[i].weight;
	if (left < m)
	    tweight[i] += tweight[left];
	if (right < m)
	    tweight[i] += tweight[right];
    }
}

#ifdef DEBUG_HEAPIFY
void check_tweights(int m, particle_info *particle) {
    int i;
    for (i = m - 1; i >= 0; --i) {
	int left = 2 * i + 1;
	int right = 2 * i + 2;
	double w = particle[i].weight;
	if (left < m)
	    w += tweight[left];
	if (right < m)
	    w += tweight[right];
	assert(fabs(w - tweight[i]) <= DW);
    }
}
#endif

int resample_logm(double scale, int m,
		  particle_info *particle,
		  int n, particle_info *newp, int sort) {
    int i;
    double best_w = 0;
    int best_i = 0;
    if (sort) {
	heapify(m, particle);
#ifdef DEBUG_HEAPIFY
	check_tweights(m, particle);
	for (i = m - 1; i > 0; --i)
	    assert (particle[i].weight <= particle[(i - 1) / 2].weight);
#endif
    } else {
	init_tweights(m, particle);
    }
#ifdef DEBUG_LOGM
    assert(tweight[0] * (1.0 - DW) <= scale &&
	   scale <= tweight[0] * (1.0 + DW));
    total_depth = 0;
#endif
    for (i = 0; i < n; i++) {
        newp[i] = *logm_weighted_sample(m, particle, tweight[0]);
        if (newp[i].weight > best_w) {
	    best_w = newp[i].weight;
	    best_i = i;
	}
    }
#ifdef DEBUG_LOGM_SEARCH
    fprintf(stderr, "%f\n", total_depth / (double) m);
#endif
    return best_i;
}

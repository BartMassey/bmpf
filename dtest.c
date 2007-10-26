/* Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

#define COUNT 10
#define TRIALS 1000000

#include <stdio.h>
#include <ziggurat/random.h>

double sample_mean[COUNT];

void sample(void) {
    int i, j;
    for (j = 0; j < TRIALS; j++) {
	double x = 0;
	for (i = 0; i < COUNT; i++) {
	    double x0 = 1.0 - pow(uniform(), 1.0 / (COUNT - i));
	    x += x0 * (1 - x);
	    sample_mean[i] += x;
	}
    }
}

double real_mean[COUNT];

int fcmp(const void *p1, const void *p2) {
    const double *f1 = p1;
    const double *f2 = p2;
    if (*f1 < *f2)
	return -1;
    if (*f1 > *f2)
	return 1;
    return 0;
}

double variate[COUNT];

void real(void) {
    int i, j;
    for (j = 0; j < TRIALS; j++) {
	for (i = 0; i < COUNT; i++)
	    variate[i] = uniform();
	qsort(variate, COUNT, sizeof(double), fcmp);
	for (i = 0; i < COUNT; i++)
	    real_mean[i] += variate[i];
    }
}

int main(int argc, char **argv) {
    int i;
    sample();
    real();
    for (i = 0; i < COUNT; i++)
	printf("%d %f %f\n", i,
	       sample_mean[i] / TRIALS, real_mean[i] / TRIALS);
    return 0;
}


/* Copryight (C) 2007 Bart Massey
   ALL RIGHTS RESERVED */

/* Gaussian PRNG based on approximate computation
   of erfinv(x). */

#ifndef _GAUSSIAN_H
#define _GAUSSIAN_H

#include <limits.h>
#include "erfinv.h"

static double gaussian(double sd) {
    long r = random();
    double y = erfinv((r & ~1) / (double)(RAND_MAX - 1));
    if (r & 1)
	return  -y * sd;
    return y * sd;
}

#endif

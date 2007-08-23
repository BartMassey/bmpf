/*
 * Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

/* Gaussian PRNG based on approximate computation
   of erfinv(x). */

#ifndef _GAUSSIAN_H
#define _GAUSSIAN_H

#include <limits.h>
#include "erfinv.h"

static float gaussian(float sd) {
    long r = random();
    float y = erfinv((r & ~1) / (float)(RAND_MAX - 1));
    if (r & 1)
	return  -y * sd;
    return y * sd;
}

#endif

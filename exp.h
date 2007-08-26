#ifndef _EXP_H
#define _EXP_H

/* Copyright (C) 2007 Bart Massey
   ALL RIGHTS RESERVED */

/* Approximate exponential function, based
   on a hack from Schraudolph
     http://citeseer.ist.psu.edu/14416.html
   XXX DANGER: works only on a known-endian machine with
   IEEE 754 floating point and that doesn't get too clever
   about optimizing things. */

#include <math.h>
#include <endian.h>

static inline double exp_(double x) {
    double exp_a = 0x100000 / M_LN2;
    double exp_c = 68243;   /* minimum mean error */
    volatile double d;
    if (x > 700 || x < -700)
	return exp(x);
    ((int *)&d)[__BYTE_ORDER==__LITTLE_ENDIAN] =
	exp_a * x + (0x3ff00000 - exp_c);
    return d;
}

#endif

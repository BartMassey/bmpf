/* Copyright (C) 2007 Bart Massey
   ALL RIGHTS RESERVED */

/* Approximate normalized inverse error function.  Uses
   piecewise cubic least-squares approximations obtained
   using GNU Octave, then hand-tweaks them.  The residual is
   a hand-tweaked exponential fit, and is horrible.  Error
   is significant; this is intended only to be used in a
   fast Gaussian-distributed PRNG, in applications where the
   error is not important. */

#ifndef _ERFINV_H
#define _ERFINV_H

#include <math.h>

static float erfinv(float x) {
    assert(0.0 <= x && x < 1.0);
    if (x <= 0.9)
	return  x * (1.1117646622106996 + x *
		  (x * 1.1887663471256378 - 0.8908018914137187));
    if (x <= 0.99)
        return  x * (2257.764033265176 + x *
	          (x * 883.953153922680 - 2444.745829241307)) -
	            694.999708131834343;
    return 1.786158859307387 + 0.004 * expf((x - 0.99) * 600);
}

#endif

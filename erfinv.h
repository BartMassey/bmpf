/* Copyright (C) 2007 Bart Massey
   ALL RIGHTS RESERVED */

/* Approximate normalized inverse error function.  Uses
   piecewise polynomial least-squares approximations
   obtained using GNU Octave.  The residual is a
   hand-tweaked exponential fit, and is horrible.  Error is
   significant; this is intended only to be used in a fast
   Gaussian-distributed PRNG, in applications where the
   error is not important. */

#ifndef _ERFINV_H
#define _ERFINV_H

#include <math.h>

static float erfinv(float x) {
    assert(0.0 <= x && x < 1.0);
    if (x <= 0.4)
        return -0.003241691 + x * 0.922235728;
    if (x <= 0.9)
        return 0.369856315 + x * (-0.598012907 + x * 1.602813513);
    if (x <= 0.99)
        return -694.991315729 + x * (2257.764033265 + x * (-2444.745829241 + x * 883.953153923));
    if (x <= 0.999)
        return -717260.870032080 + x * (2168591.882440044 + x * (-2185555.532633572 + x * 734226.973095943));
    return 1.786158859307387 + 0.004 * expf((x - 0.99) * 600);
}

#endif

#ifndef _RESAMPLE_H
#define _RESAMPLE_H

#include <zrandom.h>
#include "../bpf.h"

/* accepts a total weight, an input sample size, an input sample array,
   an output sample size, an output sample array, and a sort flag */
/* returns a highest-weighted particle index */
typedef int resample(double, int, particle_info *, int, particle_info *, int);
/* accepts a maximum number of particles input / output */
typedef void init_resample(int, int);

extern init_resample init_resample_logm;
extern resample resample_naive, resample_optimal,
       resample_logm, resample_regular;

extern struct resample_info {
    char *name;
    init_resample *f_init;
    resample *f;
} resamplers[];

#endif

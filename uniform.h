#ifndef _UNIFORM_H
#define _UNIFORM_H

#include <limits.h>

static double uniform(void) {
    return random() / (double) RAND_MAX;
}

#endif

#ifndef _UNIFORM_H
#define _UNIFORM_H

#include <limits.h>

static float uniform(void) {
    return random() / (float) RAND_MAX;
}

#endif

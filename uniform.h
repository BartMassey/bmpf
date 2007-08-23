/*
 * Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

#ifndef _UNIFORM_H
#define _UNIFORM_H

#include <limits.h>

static float uniform(void) {
    return random() / (float) RAND_MAX;
}

#endif

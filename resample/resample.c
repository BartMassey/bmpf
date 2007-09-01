/*
 * Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

#include "resample.h"

struct resample_info resamplers[] = {
    {"naive", 0, resample_naive},
    {"logm", init_resample_logm, resample_logm},
    {"optimal", 0, resample_optimal},
    {"regular", 0, resample_regular},
    {0, 0}
};

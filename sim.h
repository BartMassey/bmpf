/*
 * Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

#define BOX_DIM 20.0
#define MAX_SPEED 2.0

extern double avar;
extern double rvar;
extern double gps_var;
extern double imu_r_var;
extern double imu_a_var;

extern int fast_direction;
/* should be divisible by 4, and powers of 2 may
   be more efficient */
#define NDIRNS 1024
extern double cos_dirn[NDIRNS];

#include "sim_inline.h"

extern void init_dirn(void);
extern void init_state(state *s);
extern void update_state(state *s, double dt, int noise);

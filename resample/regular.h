/*
 * Copyright (C) 2007 Bart Massey
 * ALL RIGHTS RESERVED
 * 
 * This program is licensed under the GPL version 2 or GPL
 * version 3 or later.  Please see the file COPYING in the
 * source distribution of this software for license terms.
 */

double resample_regular(double scale,
			particle_info *particle,
			particle_info *newp) {
    int i, j;
    double u0, t = 0;
    double newscale = 0;
    if (sort) {
	/* shuffle */
	for (i = 0; i < nparticles - 1; i++) {
	    j = rand32() % (nparticles - i) + i;
	    particle_info ptmp = particle[j];
	    particle[j] = particle[i];
	    particle[i] = ptmp;
	}
    }
    /* merge */
    u0 = scale / (nparticles + 1);
    j = 0;
    for (i = 0; i < nparticles; i++ ) {
        while (t + particle[j].weight < u0 && j < nparticles)
	    t += particle[j++].weight;
#ifdef DEBUG_REGULAR
	if (j >= nparticles) {
	    fprintf(stderr, "fell off end s=%.14g t=%.14g u=%.14g\n",
		    scale, t, u0);
	    abort();
	}
#endif
	newp[i] = particle[j];
	newscale += newp[i].weight;
	u0 += scale / (nparticles + 1);
    }
    return newscale;
}

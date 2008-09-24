#include <stdio.h>
#include <math.h>
#include "exp.h"

int main() {
    double u;
    for (u = -700; u < 700; u += 0.1) {
	printf("%g %g %g\n", u, log(exp(u)), log(exp_(u)));
    }
    return 0;
}

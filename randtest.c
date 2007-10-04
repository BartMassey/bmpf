#include <stdio.h>
#include <math.h>
#include <ziggurat/random.h>

#define NBINS 500
#define N 3

int real[NBINS];
int sim[NBINS];
int simp[NBINS];

double minrand(int n) {
    int i;
    double x = uniform();
    for (i = 0; i < n; i++) {
	double y = uniform();
	if (y < x)
	    x = y;
    }
    return x;
}

int main(int argc, char **argv) {
    int i;
    int n = atoi(argv[1]);
    for (i = 0; i < n; i++) {
	double x0 = minrand(N);
	real[(int)floor(NBINS * x0)]++;
	x0 = 1.0 - pow(uniform(), 1.0 / (N + 1));
	sim[(int)floor(NBINS * x0)]++;
	x0 = polynomial(N);
	simp[(int)floor(NBINS * x0)]++;
    }
    for (i = 0; i < NBINS; i++) {
	printf("%g %d %d %d\n", i / (double)NBINS,
	       real[i], sim[i], simp[i]);
	if (real[i] == 0 && sim[i] == 0 && simp[i] == 0)
	    break;
    }
    return 0;
}

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>

static const double nsecs = 100;
static const double dt = 0.1;
#define nparticles 1000

static double avar = M_PI / 16;
static double pvar = 0.1;

typedef struct { double x, y; } ccoord;
typedef struct { double r, t; } acoord;
typedef struct { ccoord posn; acoord vel; } state;

static double uniform(void) {
    return random() / (double) INT_MAX;
}

static double gaussian(double sd) {
    double x1, w, y1;

    do {
	x1 = 2.0 * uniform() - 1.0;
	double x2 = 2.0 * uniform() - 1.0;
	w = x1 * x1 + x2 * x2;
    } while ( w >= 1.0 );

    w = sqrt( (-2.0 * log( w ) ) / w );
    y1 = x1 * w;
    return y1 * sd;
}

static void update_state(state *p, double dt) {
    double r0 = p->vel.r + gaussian(pvar);
    double t0 = p->vel.t + gaussian(avar);
    double x0 = p->posn.x + r0 * cos(t0) * dt;
    double y0 = p->posn.y - r0 * sin(t0) * dt;

    p->vel.r = r0;
    p->vel.t = t0;
    p->posn.x = x0;
    p->posn.y = y0;
}

static state vehicle = { {0, 0}, {10, 0} };

static state particle_states[2][nparticles];
static int which_particle = 0;
static state *particle = particle_states[0];

void init_particles(void) {
    int i;
    for (i = 0; i < nparticles; i++) {
	particle[i].posn.x = abs(gaussian(1));
	particle[i].posn.y = abs(gaussian(1));
	particle[i].vel.r = abs(gaussian(1));
	particle[i].vel.t = uniform() * M_PI_2;
    }
}

ccoord gps_measure(void) {
    ccoord result = vehicle.posn;
    result.x += gaussian(pvar);
    result.y += gaussian(pvar);
    return result;
}

acoord imu_measure(double dt) {
    acoord result = vehicle.vel;
    result.r += gaussian(pvar / dt);
    result.t += gaussian(avar / dt);
    return result;
}

/* XXX is this right? */
double gprob(double delta, double sd) {
    return exp(-(delta * delta * sd));
}

double gps_prob(state *particle, ccoord *gps) {
    double px = gprob(particle->posn.x - gps->x, pvar);
    double py = gprob(particle->posn.y - gps->y, pvar);
    /* XXX is this right? */
    return px * py;
}

double imu_prob(state *particle, acoord *imu) {
    double pr = gprob(particle->vel.r - imu->r, pvar);
    double pt = gprob(particle->vel.t - imu->t, avar);
    /* XXX is this right? */
    return pr * pt;
}

double sum(double a[]) {
    int i;
    double t = a[0];
    for (i = 1; i < nparticles; i++)
	t += a[i];
    return t;
}

int argmax(double a[]) {
    int i = 0, j;
    for (j = 1; j < nparticles; j++)
	if (a[j] > a[i])
	    i = j;
    return i;
}

double weight[nparticles];

state weighted_sample(void) {
    int i;
    double w = uniform();
    double t = 0;
    for (i = 0; i < nparticles; i++) {
	t += weight[i];
	if (t >= w)
	    return particle[i];
    }
    fprintf(stderr, "total %g < target %g\n", t, w);
    abort();
}

state bpf_step(ccoord *gps, acoord *imu, double dt) {
    int i;
    state *newp;
    /* update particles */
    for (i = 0; i < nparticles; i++)
	update_state(&particle[i], dt);
    /* do probabilistic weighting */
    for (i = 0; i < nparticles; i++) {
	double gp = gps_prob(&particle[i], gps);
	double ip = imu_prob(&particle[i], imu);
        weight[i] = gp * ip;
    }
    /* normalize */
    double tweight = sum(weight);
    assert(tweight > 1e-10);
    for (i = 0; i < nparticles; i++)
	weight[i] /= tweight;
    /* resample */
    newp = particle_states[!which_particle];
    for (i = 0; i < nparticles; i++)
        newp[i] = weighted_sample();
    /* find max weighted */
    state best = particle[argmax(weight)];
    /* complete */
    particle = newp;
    which_particle = !which_particle;
    return best;
}

int main() {
    double t;
    init_particles();
    for(t = 0; t <= nsecs; t += dt) {
	update_state(&vehicle, dt);
	printf("%g %g ", vehicle.posn.x, vehicle.posn.y);
	ccoord gps = gps_measure();
	acoord imu = imu_measure(dt);
	state est = bpf_step(&gps, &imu, dt);
	printf("%g %g\n", est.posn.x, est.posn.y);
    }
    return 0;
}

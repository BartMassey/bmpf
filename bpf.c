/* Copryight (C) 2007 Bart Massey
   ALL RIGHTS RESERVED */

/* Bayesian Particle Filtering demo: drive a simulated
   vehicle with a simulated IMU-ish and GPS-ish device
   around and try to track it. */

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

static state particle_buffer[2][nparticles];
static state *particle = particle_buffer[0];
static int which_particle_buffer = 0;

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

int argmax(double a[]) {
    int i = 0, j;
    for (j = 1; j < nparticles; j++)
	if (a[j] > a[i])
	    i = j;
    return i;
}

typedef struct {
    double weight, left_weight, right_weight;
    int particle;
} heapent;
static heapent heap[nparticles];

void heapify(void) {
    for (i = nparticles - 1; i >= 0; i++) {
	int this = i;
	while (1) {
	    int left = 2 * this + 1;
	    int right = 2 * this + 2;
	    double hlw, hrw;
	    int lbig, rbig, next;
	    if (left >= nparticles) {
		heap[this].left_weight = 0;
		heap[this].right_weight = 0;
		break;
	    }
	    hlw = heap[left].weight;
	    if (right >= nparticles) {
		if (hlw > heap[this].weight)
		    heap_exchange(left, i);
		heap[left].left_weight = 0;
		heap[this].left_weight = hlw;
		break;
	    }
	    hrw = heap[right].weight;
	    lbig = hlw > heap[this].weight;
	    rbig = hrw > heap[this].weight;
	    next = left;
	    if (!lbig && !rbig) {
		heap_weights(this);
		break;
	    }
	    if (rbig && (!lbig || hrw > hlw))
		next = right;
	    heap_exchange(next, this);
	    this = next;
	}
    }
}

state weighted_sample(double tweight) {
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
    double tweight;
    /* update particles */
    for (i = 0; i < nparticles; i++)
	update_state(&particle[i], dt);
    /* do probabilistic weighting */
    for (i = 0; i < nparticles; i++) {
	double gp = gps_prob(&particle[i], gps);
	double ip = imu_prob(&particle[i], imu);
	heap[i].particle = i;
        heap[i].weight = gp * ip;
    }
    /* heapify */
    heapify();
    /* get normalization */
    tweight = heap[0].weight +
	      heap[0].left_weight +
	      heap[0].right_weight;
    assert(tweight > 1e-10);
    /* resample */
    /* XXX shouldn't need to copy these, just pointers.  uggh. */
    newp = particle_buffers[!which_particle_buffer];
    for (i = 0; i < nparticles; i++)
        newp[i] = weighted_sample(tweight);
    /* find max weighted */
    state best = particle[argmax(weight)];
    /* complete */
    particle = newp;
    which_particle_buffer = !which_particle_buffer;
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

#ifndef _BPF_H
#define _BPF_H

typedef struct ccoord { double x, y; } ccoord;
typedef struct acoord { double r, t; } acoord;
typedef struct state { ccoord posn; acoord vel; } state;
typedef struct { state state; double weight; } particle_info;

#endif

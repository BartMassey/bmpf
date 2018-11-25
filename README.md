# BMPF: Bayesian Mega-Particle Filtering
Copyright (c) 2007 Bart Massey

This is a Bayesian Particle Filter implementation with a
vehicle simulator and various evaluation and benchmarking
tests.

## Background and Status

I've developed an algorithm for perfect linear-time weighted
resampling of a distribution.  This enables fast Bayesian
Particle Filtering with extremely high particle densities,
without concern for correct resampling.  The algorithm is
not a big advance on the state of the art, but it seems like
a nice piece of work.

Right now, I can run BPF on a toy problem in 0.1 sec steps
at about 140,000 particles in "real time", on my fast Core
II Duo box.  I call the code "Mega-Particle" because I am
already achieving about 1.4M particle updates per second on
a fast machine&mdash;resampling at each update.  However,
resampling is not the bottleneck step now, making this
number somewhat arbitrary from the point of view of this
research.

I've written a large draft [paper](ltrs.pdf) on my work so
far, as well as a much shorter paper for ICASSP 2008.

To build this code you'll currently also need my
[Ziggurat](http://github.com/BartMassey/ziggurat)
code&mdash;switching to it removed the first constant-factor
bottleneck in my BMPF implementation, approximately doubling
the speed.  You may also find it useful in general.  You
might also want to take a look at the fast approximate
`exp()` I found and re-implemented, which removed another
bottleneck.  I've also done work on table-driven `sin()` and
`cos()` that attacks that performance bottleneck, and
cleaned up and optimized the C code.

## Using the Code

The normal workflow here is ill-documented and complex.

* Start by running `make` with no arguments.  The default
  targets in the Makefile will be the vehicle simulator in
  vehicle and the BPF code in bpf.

* Generate the simulated vehicle trace with `make
  vehicle.dat`.

* Now actually run the benchmarks: `sh benchmark.sh`.
  Do this on an unloaded machine, and wander away for
  an hour or more.

* Save the benchmarks with "sh savebench.sh".  This
  will copy the benchmark outputs and timings from
  `benchtmp/` to `bench/` and simultaneously create the
  `bench/*.plot` files needed for plotting things.

* Format the paper with "make ltrs.pdf".  (The name "ltrs"
  is for "linear-time resampling", and is way out of date.
  C'est la vie.)  This should cause all the plots used in
  the paper to be built.

* The other shell scripts here are just utility scripts to
  use in experimentation.  Note in particular
  xplottimes.sh, which is useful for more interactive
  plots to your X screen.  The shell scripts are horribly
  undocumented: I should fix that sometime.

## License

This work is distributed under the GPL version 2 or GPL
version 3 or later.  See the file COPYING in this directory
for the conditions of this license.

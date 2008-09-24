This is a Bayesian Particle Filter implementation with a
vehicle simulator and various evaluation and benchmarking
tests.  For more information, please see

  http://wiki.cs.pdx.edu/bartforge/bmpf


This work is distributed under the GPL version 2 or GPL
version 3 or later.  See the file COPYING in this directory
for the conditions of this license.


The normal workflow here is ill-documented and complex.

  * Start by running "make" with no arguments.  The default
    targets in the Makefile will be the vehicle simulator in
    vehicle and the BPF code in bpf.

  * Generate the simulated vehicle trace with "make
    vehicle.dat".

  * Now actually run the benchmarks: "sh benchmark.sh".
    Do this on an unloaded machine, and wander away for
    an hour or more.

  * Save the benchmarks with "sh savebench.sh".  This
    will copy the benchmark outputs and timings from
    benchtmp/ to bench/ and simultaneously create the
    bench/*.plot files needed for plotting things.    

  * Format the paper with "make ltrs.pdf".  (The name "ltrs"
    is for "linear-time resampling", and is way out of date.
    C'est la vie.)  This should cause all the plots used in
    the paper to be built.

  * The other shell scripts here are just utility scripts to
    use in experimentation.  Note in particular
    xplottimes.sh, which is useful for more interactive
    plots to your X screen.  The shell scripts are horribly
    undocumented: I should fix that sometime.


Notes for me and other interested parties:

  * I checked ltrs.pdf into git because it makes it
    easy to post it on bartforge via gitweb.  Leave it alone.

Have fun!

Bart Massey <bart@cs.pdx.edu>
23 September 2008

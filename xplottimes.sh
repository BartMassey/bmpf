#!/bin/sh
TMP=/tmp/xplottimes-$$.plot
trap "rm -f $TMP" 0
sh plottimes.sh -x "$@" > $TMP
gnuplot $TMP

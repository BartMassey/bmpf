#!/bin/sh
# Copyright (C) 2007 Bart Massey
# ALL RIGHTS RESERVED
# 
# [This program is licensed under the GPL version 2 or GPL version 3 or later.]
# Please see the file COPYING in the source
# distribution of this software for license terms.

# try at most this many particles
maxp=150000
# if a run takes longer than this, stop this algorithm
maxt=120.0
# make sure we accumulate at least this much runtime for each trial
rept=1.0

plottmp=/tmp/benchmark.$$
trap "rm -f $plottmp" 0
if [ $# -eq 0 ] ; then set `awk '{print $1;}' <algorithms` ; fi
for a in "$@"
do
  p=10
  incr=$p
  echo $a
  # warm up the caches
  (time ./bpf $p $a >/dev/null) 2>/dev/null
  while [ $p -le $maxp ]
  do
    timesfile=benchtmp/$a-$p.time
    datfile=benchtmp/$a-$p.dat
    plotfile=benchtmp/$a-$p.pline
    : >$plottmp
    tt=0
    while nickle -e "if($tt < $rept) exit(0); else exit(1);;"
    do
      (time ./bpf $p $a >$datfile) 2>$timesfile
      t=`awk '$1=="real" {
	 i = index($2, "m")
	 m = substr($2, 1, i - 1)
	 s = substr($2, i + 1, length($2) - i + 1)
	 print 60 * m + s
	 }' < $timesfile`
      echo $p $t >>$plottmp
      tt=`nickle -e "$tt + $t"`
    done
    sort -k 1 -n $plottmp | awk '
      { p=$1; t[++n]=$2; }
      END { print p, t[int((n + 1) / 2)]; }' >$plotfile
    t=`awk '{print $2;}' < $plotfile`
    if nickle -e "if($t > $maxt) exit(0); else exit(1);;" \
       >/dev/null ; then break ; fi
    if [ $p -eq 50000 ] ; then
      incr=25000 ; echo $a $p
    elif [ $p -eq 10000 ] ; then
      incr=5000 ; echo $a $p
    elif [ $p -eq 1000 ] ; then
      incr=500 ; echo $a $p
    elif [ $p -eq 100 ] ; then
      incr=50 ; echo $a $p
    fi
    p=`expr $p + $incr`
  done
done

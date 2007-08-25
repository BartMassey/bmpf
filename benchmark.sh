#!/bin/sh
# Copyright (C) 2007 Bart Massey
# ALL RIGHTS RESERVED
# 
# [This program is licensed under the GPL version 2 or GPL version 3 or later.]
# Please see the file COPYING in the source
# distribution of this software for license terms.
for a in optimal logm logmsort naivesort naive
do
  p=100
  incr=100
  while [ $p -le 150000 ]
  do
    timesfile=benchtmp/$a-$p.time
    datfile=benchtmp/$a-$p.dat
    plotfile=benchtmp/$a-$p.plot
    (time ./bpf $p $a >$datfile) 2>$timesfile
    t=`awk '$1=="real" {
       i = index($2, "m")
       m = substr($2, 1, i - 1)
       s = substr($2, i + 1, length($2) - i + 1)
       print 60 * m + s
       }' < $timesfile`
    echo $p $t >$plotfile
    if nickle -e "if($t > 150) exit(0); else exit(1);;" \
       >/dev/null ; then break ; fi
    if [ $p -ge 500 ] ; then incr=500 ; fi
    if [ $p -ge 10000 ] ; then incr=5000 ; fi
    if [ $p -ge 50000 ] ; then incr=25000 ; fi
    p=`expr $p + $incr`
  done
done

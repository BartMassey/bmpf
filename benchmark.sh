#!/bin/sh
# Copyright (C) 2007 Bart Massey
# ALL RIGHTS RESERVED
# 
# [This program is licensed under the GPL version 2 or GPL version 3 or later.]
# Please see the file COPYING in the source
# distribution of this software for license terms.
if [ $# -eq 0 ] ; then set `awk '{print $1;}' <algorithms` ; fi
for a in "$@"
do
  p=10
  incr=10
  echo $a
  while [ $p -le 150000 ]
  do
    timesfile=benchtmp/$a-$p.time
    datfile=benchtmp/$a-$p.dat
    plotfile=benchtmp/$a-$p.pline
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

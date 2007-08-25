#!/bin/sh
# Copyright (C) 2007 Bart Massey
# ALL RIGHTS RESERVED
# 
# [This program is licensed under the GPL version 2 or GPL version 3 or later.]
# Please see the file COPYING in the source
# distribution of this software for license terms.
for a in optimal naive logm naivesort logmsort
do
  t=100
  incr=100
  while [ $t -le 10000 ]
  do
    if [ $t -ge 500 ] ; then incr=500 ; fi
    (time ./bpf $t $a >benchtmp/$a-$t.dat) 2>benchtmp/$a-$t.time
    t=`expr $t + $incr`
  done
done
for a in optimal logm logmsort
do
  t=10000
  incr=5000
  while [ $t -le 150000 ]
  do
    if [ $t -ge 50000 ] ; then incr=25000 ; fi
    (time ./bpf $t $a >benchtmp/$a-$t.dat) 2>benchtmp/$a-$t.time
    t=`expr $t + $incr`
  done
done

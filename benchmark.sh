#!/bin/sh
# Copyright (C) 2007 Bart Massey
# ALL RIGHTS RESERVED
# 
# [This program is licensed under the GPL version 2 or GPL version 3 or later.]
# Please see the file COPYING in the source
# distribution of this software for license terms.
for a in optimal naive
do
  for t in 100 500 1000 5000 10000
  do
    (time ./bpf $t $a >$a-$t.dat) 2>$a-$t.time
  done
done

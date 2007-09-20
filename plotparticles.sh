#!/bin/sh
# Copyright (C) 2007 Bart Massey
# ALL RIGHTS RESERVED
# 
# This program is licensed under the GPL version 2 or GPL
# version 3 or later.  Please see the file COPYING in the
# source distribution of this software for license terms.
for TIME in `seq 0 10 100`
do
  DAT=benchtmp/particles-$TIME.dat
  EPS=benchtmp/particles-$TIME.eps
  cat <<EOF |
  set terminal postscript eps
  set title "Relative particle positions, t=${TIME}s"
  plot [-3:3] [-3:3] "$DAT" notitle
EOF
  gnuplot >"$EPS"
done

#!/bin/sh
# Copyright (C) 2007 Bart Massey
# ALL RIGHTS RESERVED
# 
# This program is licensed under the GPL version 2 or GPL
# version 3 or later.  Please see the file COPYING in the
# source distribution of this software for license terms.
E=""
if [ $# -gt 1 ]
then
    E="every $2"
fi
cat <<EOF
set terminal postscript eps
plot "$1" using 1:2 $E with points title 'actual vehicle track', \
     "" using 3:4 $E with points title 'min. est. vehicle track', \
     "" using 5:6 $E with points title 'avg. est. vehicle track';
EOF

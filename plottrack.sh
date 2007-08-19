#!/bin/sh
cat <<EOF
set terminal postscript eps
plot "$1" using 1:2 with points title 'actual vehicle track', \
     "$1" using 3:4 with points title 'estimated vehicle track'
EOF

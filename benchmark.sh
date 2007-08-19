#!/bin/sh
for a in optimal naive
do
  for t in 100 500 1000 5000 10000
  do
    (time ./bpf $t $a >$a-$t.dat) 2>$a-$t.time
  done
done

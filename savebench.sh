#!/bin/sh
( mv -f benchtmp/* bench 2>/dev/null ; exit 0 )
while read a t
do
  cat bench/$a-*.pline | sort -n >bench/$a.plot
done <algorithms

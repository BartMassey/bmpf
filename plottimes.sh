#!/bin/sh
cat <<'EOF'
set terminal postscript eps
set ylabel 'time (s)'
set xlabel 'particles'
plot \
     '-' with linespoints title 'BPF with naive resampling', \
     '-' with linespoints title 'BPF with naive resampling and presort', \
     '-' with linespoints title 'BPF with logm resampling and presort', \
     '-' with linespoints title 'BPF with logm resampling', \
     '-' with linespoints title 'BPF with optimal resampling'
EOF
for a in naive naivesort logmsort logm optimal
do
  for f in $a-*.time
  do
    n=`echo $f | sed -e "s/^$a-//" -e 's/\.time$//'`
    t=`awk '$1=="real" {
       i = index($2, "m")
       m = substr($2, 1, i - 1)
       s = substr($2, i + 1, length($2) - i + 1)
       print 60 * m + s
       }' < $f`
    echo $n $t
  done | sort -n
  echo 'e'
done
#echo 'pause -1'

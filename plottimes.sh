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
  cat $a-*.plot | sort -n
  echo 'e'
done
#echo 'pause -1'

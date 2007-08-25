#!/bin/sh
cat <<'EOF'
set terminal postscript eps
set ylabel 'time (secs for 1000 steps)'
set xlabel 'particles'
plot \
     '-' with linespoints title 'BPF with naive resampling', \
     '-' with linespoints title 'BPF with naive resampling and presort', \
     '-' with linespoints title 'BPF with logm resampling and presort', \
     '-' with linespoints title 'BPF with logm resampling', \
     '-' with linespoints title 'BPF with optimal resampling', \
     '-' with linespoints title 'BPF with no resampling'
EOF
for a in naive naivesort logmsort logm optimal none
do
  cat $a.plot
  echo 'e'
done
#echo 'pause -1'

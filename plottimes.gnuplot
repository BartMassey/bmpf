set terminal postscript eps
set ylabel 'time (s)'
set xlabel 'particles'
plot '-' with linespoints title 'BPF with naive resampling', \
     '-' with linespoints title 'BPF with optimal resampling'
100 0.12
500 0.99
1000 2.67
5000 42.68
10000 160.67
e
100 0.13
500 0.62
1000 1.24
5000 6.14
10000 12.10
e

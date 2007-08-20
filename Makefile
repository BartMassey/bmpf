CC = gcc
CFLAGS = -g -Wall -O4
EPS = bars.eps track-naive-100.eps track-optimal-100.eps times.eps

all: bpf ltrs.dvi

bpf: bpf.c
	$(CC) $(CFLAGS) -o bpf bpf.c -lm

ltrs.dvi: ltrs.tex $(EPS)
	latex ltrs
	latex ltrs

bars.eps: plotbars.5c
	nickle plotbars.5c | gnuplot > bars.eps

times.eps: plottimes.gnuplot
	gnuplot plottimes.gnuplot > times.eps

track-naive-100.eps: plottrack.sh bench/naive-100.dat
	sh plottrack.sh bench/naive-100.dat > track-naive-100.eps

track-optimal-100.eps: plottrack.sh bench/optimal-100.dat
	sh plottrack.sh bench/optimal-100.dat > track-optimal-100.eps

clean:
	-rm -f bars.eps ltrs.dvi bpf

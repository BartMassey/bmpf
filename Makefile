CC = gcc
CFLAGS = -g -Wall -O4

all: ltrs.dvi bpf

ltrs.dvi: ltrs.tex bars.eps track-naive-100.eps track-optimal-100.eps
	latex ltrs
	latex ltrs

bars.eps: plotbars.5c
	nickle plotbars.5c | gnuplot > bars.eps

track-naive-100.eps: plottrack.sh bench/naive-100.dat
	sh plottrack.sh bench/naive-100.dat > track-naive-100.eps

track-optimal-100.eps: plottrack.sh bench/optimal-100.dat
	sh plottrack.sh bench/optimal-100.dat > track-optimal-100.eps

bpf: bpf.c
	$(CC) $(CFLAGS) -o bpf bpf.c -lm

clean:
	-rm -f bars.eps ltrs.dvi bpf

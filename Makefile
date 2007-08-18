CC = gcc
CFLAGS = -g -O2 -Wall

all: ltrs.dvi bpf

ltrs.dvi: ltrs.tex bars.eps
	latex ltrs
	latex ltrs

bars.eps: plotbars.5c
	nickle plotbars.5c | gnuplot > bars.eps

bpf: bpf.c
	$(CC) $(CFLAGS) -o bpf bpf.c -lm

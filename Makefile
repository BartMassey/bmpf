# Copyright (C) 2007 Bart Massey
# ALL RIGHTS RESERVED
# 
# This program is licensed under the GPL version 2 or GPL
# version 3 or later.  Please see the file COPYING in the
# source distribution of this software for license terms.

CC = gcc
#CFLAGS = -g -Wall -O4 -pg
#LIBS = -L/local/lib/ziggurat -lrandom_p -lm_p
CFLAGS = -g -Wall -O4
LIBS = -L/local/lib/ziggurat -lrandom -lm
EPS = bars.eps track-naive-100.eps track-optimal-100.eps times.eps

bpf: bpf.c uniform.h erfinv.h gaussian-erfinv.h
	$(CC) $(CFLAGS) -o bpf bpf.c $(LIBS)

all: bpf ltrs.dvi

ltrs.dvi: ltrs.tex $(EPS)
	latex ltrs
	latex ltrs

bars.eps: plotbars.5c
	nickle plotbars.5c | gnuplot > bars.eps

times.eps: plottimes.gnuplot
	gnuplot plottimes.gnuplot > times.eps

track-naive-100.eps: plottrack.sh bench/naive-100.dat
	sh plottrack.sh bench/naive-100.dat | \
	  gnuplot > track-naive-100.eps

track-optimal-100.eps: plottrack.sh bench/optimal-100.dat
	sh plottrack.sh bench/optimal-100.dat | \
	  gnuplot > track-optimal-100.eps

clean:
	-rm -f *.eps ltrs.dvi bpf gmon.out

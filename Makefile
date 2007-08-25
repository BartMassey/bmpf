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
EPS = bars.eps track-naive-100.eps track-optimal-100.eps \
      times.eps timeszoom.eps timeszoom2.eps
PLOTS = bench/regular.plot bench/optimal.plot bench/logm.plot \
        bench/logmsort.plot bench/naivesort.plot bench/naive.plot

bpf: bpf.c uniform.h erfinv.h gaussian-erfinv.h
	$(CC) $(CFLAGS) -o bpf bpf.c $(LIBS)

all: bpf ltrs.pdf

ltrs.pdf: ltrs.dvi
	dvitopdf ltrs

ltrs.dvi: ltrs.tex $(EPS)
	latex ltrs
	latex ltrs

bars.eps: plotbars.5c
	nickle plotbars.5c | gnuplot > bars.eps

times.eps: plottimes.sh $(PLOTS)
	sh plottimes.sh bench | gnuplot > times.eps

timeszoom.eps: plottimes.sh $(PLOTS)
	sh plottimes.sh -p 700 bench | gnuplot > timeszoom.eps

timeszoom2.eps: plottimes.sh $(PLOTS)
	sh plottimes.sh -p 100 bench | gnuplot > timeszoom2.eps

track-naive-100.eps: plottrack.sh bench/naive-100.dat
	sh plottrack.sh bench/naive-100.dat | \
	  gnuplot > track-naive-100.eps

track-optimal-100.eps: plottrack.sh bench/optimal-100.dat
	sh plottrack.sh bench/optimal-100.dat | \
	  gnuplot > track-optimal-100.eps

clean:
	-rm -f *.eps ltrs.dvi ltrs.log ltrs.ps bpf gmon.out

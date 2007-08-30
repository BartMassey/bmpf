# Copyright (C) 2007 Bart Massey
# ALL RIGHTS RESERVED
# 
# This program is licensed under the GPL version 2 or GPL
# version 3 or later.  Please see the file COPYING in the
# source distribution of this software for license terms.

CC = gcc
#CFLAGS = -g -Wall -O4 -pg -fprofile-arcs -ftest-coverage
#LIBS = -L/local/lib/ziggurat -lrandom_p -lm_p
CFLAGS = -g -Wall -O4
LIBS = -L/local/lib/ziggurat -lrandom -lm
TIMESEPS = times.eps timeszoom.eps timeszoom2.eps
EPS = bars.eps track-naive-100.eps track-optimal-100.eps $(TIMESEPS)
      
PLOTS = bench/regular.plot bench/optimal.plot bench/logm.plot \
        bench/logmsort.plot bench/naivesort.plot bench/naive.plot
RESAMPLERS = resample/resample.o \
	     resample/logm.o resample/naive.o \
             resample/optimal.o resample/regular.o

bpf: bpf.c exp.h $(RESAMPLERS)
	$(CC) $(CFLAGS) -Wno-strict-aliasing -o bpf bpf.c $(RESAMPLERS) $(LIBS)

$(RESAMPLERS): bpf.h resample/resample.h

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

times: $(TIMESEPS)

track-naive-100.eps: plottrack.sh bench/naive-100.dat
	sh plottrack.sh bench/naive-100.dat | \
	  gnuplot > track-naive-100.eps

track-optimal-100.eps: plottrack.sh bench/optimal-100.dat
	sh plottrack.sh bench/optimal-100.dat | \
	  gnuplot > track-optimal-100.eps

realclean: clean docclean

docclean:
	-rm -f *.eps ltrs.dvi ltrs.log ltrs.ps 

clean:
	-rm -f bpf gmon.out $(RESAMPLERS)

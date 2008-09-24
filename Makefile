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
TIMESEPS = times.eps timeszoom2.eps
TIMESPDF = times.pdf timeszoom2.pdf
EPS = bars.eps track-naive-100.eps track-optimalsort-100.eps $(TIMESEPS)
PDF = bars.pdf track-naive-100.pdf track-optimalsort-100.pdf $(TIMESPDF)
      
PLOTS = bench/regular.plot bench/regularsort.plot \
        bench/optimal.plot bench/optimalsort.plot \
        bench/logm.plot bench/logmsort.plot \
        bench/naivesort.plot bench/naive.plot
RESAMPLERS = resample/resample.o \
	     resample/logm.o resample/naive.o \
             resample/optimal.o resample/regular.o

.SUFFIXES: .eps .pdf

.eps.pdf:
	epstopdf $*.eps

all: bpf vehicle

bpf: bpf.o sim.o $(RESAMPLERS)
	$(CC) $(CFLAGS) -Wno-strict-aliasing -o bpf \
	    bpf.o sim.o $(RESAMPLERS) $(LIBS)

vehicle: vehicle.o sim.o
	$(CC) $(CFLAGS) -Wno-strict-aliasing -o vehicle \
	    vehicle.o sim.o $(LIBS)

bpf.o sim.o vehicle.o: bpf.h sim.h sim_inline.h exp.h

randtest: randtest.c
	$(CC) $(CFLAGS) -o randtest randtest.c $(LIBS)

$(RESAMPLERS): bpf.h resample/resample.h

all: bpf ltrs.pdf

ltrs.pdf: ltrs.tex ltrs.bbl $(PDF)
	pdflatex ltrs
	pdflatex ltrs

ltrs.dvi: ltrs.tex ltrs.bbl $(EPS)
	latex ltrs
	latex ltrs

ltrs.bbl: ltrs.bib $(EPS)
	latex ltrs
	bibtex ltrs

bars.eps: plotbars.5c
	nickle plotbars.5c | gnuplot > bars.eps

times.eps: plottimes.sh $(PLOTS)
	sh plottimes.sh | gnuplot > times.eps

timeszoom.eps: plottimes.sh $(PLOTS)
	sh plottimes.sh -p 500 \
         optimalsort optimal regularsort regular | \
        gnuplot > timeszoom.eps

timeszoom2.eps: plottimes.sh $(PLOTS)
	sh plottimes.sh -p 100 | gnuplot > timeszoom2.eps

times: $(TIMESPDF)

track-naive-100.eps: plottrack.sh bench/naive-100.dat
	sh plottrack.sh bench/naive-100.dat | \
	  gnuplot > track-naive-100.eps

track-optimalsort-100.eps: plottrack.sh bench/optimalsort-100.dat
	sh plottrack.sh bench/optimalsort-100.dat | \
	  gnuplot > track-optimalsort-100.eps

realclean: clean docclean

docclean:
	-rm -f $(EPS) $(PDF) \
               ltrs.pdf ltrs.dvi ltrs.log \
               ltrs.ps ltrs.blg ltrs.aux

clean:
	-rm -f vehicle vehicle.o bpf bpf.o sim.o \
               randtest vehicle.out gmon.out exp \
               $(RESAMPLERS)

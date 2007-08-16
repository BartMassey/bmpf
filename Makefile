CC = gcc
CFLAGS = -g -O2 -Wall

bpf: bpf.c
	$(CC) $(CFLAGS) -o bpf bpf.c -lm

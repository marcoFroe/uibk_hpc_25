CC = mpicc
CFLAGS = -std=gnu11 -Wall -Wextra -lm -O3 -ffast-math -march=native -g3
LDFLAGS =

.PHONY: all
all: mandelbrot_mpi

.PHONY: clean
clean:
	rm -f mandelbrot_mpi mandelbrot_mpi.png *.out

.PHONY: debug
debug: CFLAGS += -fsanitize=address -fno-omit-frame-pointer
debug: all

mandelbrot_mpi: mandelbrot_mpi.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<
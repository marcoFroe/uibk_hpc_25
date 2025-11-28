CC=mpicc
CFLAGS=-std=gnu11 -Wall -Wextra -lm -O3 -ffast-math -march=native -g3

.PHONY: all
all: mandelbrot_mpi

.PHONY: clean
clean:
	$(RM) mandelbrot_mpi mandelbrot_mpi.png

mandelbrot_mpi: mandelbrot_mpi.c

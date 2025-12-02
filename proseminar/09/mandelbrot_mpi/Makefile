CC = mpicc
CFLAGS = -std=gnu11 -Wall -Wextra -lm -O3 -ffast-math -march=native -g3
LDFLAGS =

.PHONY: all
all: mandelbrot_mpi mandelbrot_mpi_opt-1 mandelbrot_mpi_opt-2

.PHONY: clean
clean:
	rm -f mandelbrot_mpi mandelbrot_mpi_opt-1 mandelbrot_mpi_opt-2 mandelbrot_mpi.png *.out

.PHONY: debug
debug: CFLAGS += -fsanitize=address -fno-omit-frame-pointer
debug: all

mandelbrot_mpi: mandelbrot_mpi.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

mandelbrot_mpi_opt-1: mandelbrot_mpi_opt-1.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

mandelbrot_mpi_opt-2: mandelbrot_mpi_opt-2.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

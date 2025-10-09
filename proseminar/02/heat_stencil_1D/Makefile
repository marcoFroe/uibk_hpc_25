CC=mpicc
CFLAGS=-O3 -std=gnu11 -Wall -Wextra -pedantic

.PHONY: all
all: heat_stencil_1D_seq heat_stencil_1D_mpi

heat_stencil_1D_seq: heat_stencil_1D_seq.c
	@$(CC) $(CFLAGS) $< -o heat_stencil_1D_seq

heat_stencil_1D_mpi: heat_stencil_1D_mpi.c
	@$(CC) $(CFLAGS) $< -o heat_stencil_1D_mpi

.PHONY: clean
clean:
	@rm heat_stencil_1D_seq
	@rm heat_stencil_1D_mpi

.PHONY: run
run: all
	@echo "Sequential:"
	@./heat_stencil_1D_seq
	@echo
	@echo "MPI:"
	@./heat_stencil_1D_mpi
	@echo
CC=mpicc
CFLAGS=-O3 -std=gnu11 -Wall -Wextra -pedantic

TARGETS= pi_seq pi_mpi

.PHONY: all 
all: $(TARGETS)

%: %.c
	@$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	@$(RM) $(TARGETS)


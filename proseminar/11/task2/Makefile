CC=mpicc
CFLAGS= -Ofast -std=gnu11 -Wall -Wextra -pedantic

TARGETS= pi_mpi

.PHONY: all 
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(TARGETS)


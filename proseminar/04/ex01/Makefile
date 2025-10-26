CC=mpicc
CFLAGS=-std=gnu11 -Wall -Wextra

TARGETS= heat_stencil_2D_seq # heat_stencil_2D_mpi

.PHONY: all
all: $(TARGETS)

.PHONY: all 
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(TARGETS)

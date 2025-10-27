CC=mpicc
CFLAGS= -ofast -std=gnu11 -Wall -Wextra

TARGETS= heat_stencil_2D_seq heat_stencil_2D_mpi heat_stencil_2D_seq_no_if

.PHONY: all
all: $(TARGETS)

.PHONY: all 
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(TARGETS)

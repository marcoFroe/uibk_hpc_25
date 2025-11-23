# generated using ChatGPT
CC=mpicc
CFLAGS=-std=gnu11 -Wall -Wextra -Wpedantic -Ofast

SRC=$(wildcard *.c)
TARGETS=$(SRC:.c=)

.PHONY: all clean clean_files

# Default target: build all executables
all: $(TARGETS)

# Generic build rule: compile each .c file into an executable
%: %.c
	$(CC) $(CFLAGS) $< -o $@

# Clean rule: remove all generated executables
clean: clean_files
	rm -f $(TARGETS) *.out
	
clean_files:	
	 (cd $(SCRATCH)/hpc_08; rm -f *.out; cd -)
# generated using ChatGPT
CC=mpicc
CFLAGS=-std=gnu11 -Wall -Wextra -Wpedantic -Ofast -lm -DPRINT_SIM

SRC=$(wildcard *.c)
TARGETS=$(SRC:.c=)

.PHONY: all clean debug

# Default target: build all executables
all: $(TARGETS)

# Generic build rule: compile each .c file into an executable
%: %.c
	$(CC) $(CFLAGS) $< -o $@

# Clean rule: remove all generated executables
clean:
	rm -f $(TARGETS) *.out

debug: CFLAGS=-g -O0 -Wall -Wextra -Wpedantic -lm -DPRINT_SIM
debug: all
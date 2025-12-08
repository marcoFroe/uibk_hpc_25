SRC=$(wildcard *.chpl)
TARGETS=$(SRC:.chpl=)

.PHONY: all

all: montecarlo matrix_mul

montecarlo: montecarlo.chpl
	chpl $^ -o $@

matrix_mul: matrix_mul.chpl
	chpl --set blasImpl=off --set lapackImpl=off $^ -o $@

clean:
	rm -f $(TARGETS) *.out

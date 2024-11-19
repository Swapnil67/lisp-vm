CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LIBS=

.PHONY: all
all: ebasm bmi

ebasm: ebasm.c bm.c
	$(CC) $(CFLAGS) -o ebasm ebasm.c $(LIB)

bmi: bmi.c bm.c
	$(CC) $(CFLAGS) -o bmi bmi.c $(LIB)

.PHONY: examples
examples: ./examples/fib.bm

./examples/fib.bm: ./examples/fib.ebasm
	./ebasm ./examples/fib.ebasm ./examples/fib.bm


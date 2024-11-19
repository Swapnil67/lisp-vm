CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LIBS=

.PHONY: all
all: basm bme

basm: ./src/basm.c ./src/bm.c
	$(CC) $(CFLAGS) -o basm ./src/basm.c $(LIB)

bme: ./src/bme.c ./src/bm.c
	$(CC) $(CFLAGS) -o bme ./src/bme.c $(LIB)

.PHONY: examples
examples: ./examples/fib.bm

./examples/fib.bm: basm ./examples/fib.basm
	./basm ./examples/fib.basm ./examples/fib.bm


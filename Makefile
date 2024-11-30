CFLAGS=-Wall -Wextra -Wswitch-enum -Wmissing-prototypes -std=c11 -pedantic
LIBS=

EXAMPLES=$(patsubst %.basm,%.bm,$(wildcard ./examples/*.basm))

.PHONY: all
all: basm bme debasm

basm: ./src/basm.c ./src/bm.h
	$(CC) $(CFLAGS) -o basm ./src/basm.c $(LIB)

bme: ./src/bme.c ./src/bm.h
	$(CC) $(CFLAGS) -o bme ./src/bme.c $(LIB)

debasm: ./src/debasm.c ./src/bm.h
	$(CC) $(CFLAGS) -o debasm ./src/debasm.c $(LIBS)

.PHONY: examples
examples: $(EXAMPLES)

%.bm: %.basm basm
	./basm $< $@

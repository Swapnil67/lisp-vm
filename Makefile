CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LIBS=

bm: main.c
	$(CC) $(CFLAGS) -o bm main.c $(LIB)

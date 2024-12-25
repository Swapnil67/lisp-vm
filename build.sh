
#!/bin/sh

set -xe

CC=${CC:=/usr/bin/cc}
CFLAGS="-Wall -Wextra -Wswitch-enum -Wmissing-prototypes -Wconversion -fno-strict-aliasing -std=c11 -pedantic"
LIBS=
 
$CC $CFLAGS -o basm ./src/basm.c $LIBS
$CC $CFLAGS -o bme ./src/bme.c $LIBS
$CC $CFLAGS -o debasm ./src/debasm.c $LIBS

# Bake executables via xxd
$CC $CFLAGS -o xxd ./src/xxd.c $LIBS
./xxd -i src/natives.asm > src/natives.asm.h

$CC $CFLAGS -o basm2amd64 ./src/basm2amd64.c $LIBS

for example in `find examples/ -name \*.basm | sed "s/\.basm//"`; do
    # cpp -P "$example.basm" > "$example.basm.pp"
    ./basm "$example.basm" "$example.bm"
    # rm -r "$example.basm.pp"
done



#!/bin/sh

set -xe

CC=${CC:=/usr/bin/cc}
CFLAGS="-Wall -Wextra -Wswitch-enum -Wmissing-prototypes -Wconversion -fno-strict-aliasing -std=c11 -pedantic"
LIBS=

mkdir -p ./build/examples
mkdir -p ./build/bin

$CC $CFLAGS -o ./build/bin/basm ./src/basm.c $LIBS
$CC $CFLAGS -o ./build/bin/bme ./src/bme.c $LIBS
$CC $CFLAGS -o ./build/bin/bmr ./src/bmr.c $LIBS
$CC $CFLAGS -o ./build/bin/debasm ./src/debasm.c $LIBS
$CC $CFLAGS -o ./build/bin/basm2amd64 ./src/basm2amd64.c $LIBS

for example in `find examples/ -name \*.basm | sed "s/\.basm//"`; do
    ./build/bin/basm "$example.basm" "./build/$example.bm"
done

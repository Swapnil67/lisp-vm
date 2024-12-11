
#!/bin/sh

set -xe

./build.sh

./basm2amd64 ./examples/halt.basm > halt.asm

nasm -felf64 halt.asm
ld -o halt halt.o

nasm -felf64 print_i64.asm
ld -o print_i64 print_i64.o

nasm -felf64 test.asm
ld -o test test.o

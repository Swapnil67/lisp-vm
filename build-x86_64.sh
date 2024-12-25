
#!/bin/sh

set -xe

./build.sh

./basm2amd64 ./examples/fib.basm > ./build/examples/fib.asm
nasm -felf64 -g ./build/examples/fib.asm -o ./build/examples/fib.o
ld -o ./build/examples/fib.exe ./build/examples/fib.o

./basm2amd64 ./examples/123i.basm > ./build/examples/123i.asm
nasm -felf64 -g ./build/examples/123i.asm -o ./build/examples/123i.o
ld -o ./build/examples/123i.exe ./build/examples/123i.o



#!/bin/sh

set -xe

./build.sh

./basm2amd64 ./examples/123i.basm > ./examples/123i.asm
nasm -felf64 -g ./examples/123i.asm -o ./examples/123i.o
ld -o ./examples/123i.exe ./examples/123i.o


./basm2amd64 ./examples/fib.basm > ./examples/fib.asm
nasm -felf64 -g ./examples/fib.asm -o ./examples/fib.o
ld -o ./examples/fib.exe ./examples/fib.o


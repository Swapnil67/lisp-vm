
#!/bin/sh

set -xe

./build.sh

./basm2amd64 ./examples/123i.basm > 123i.asm

nasm -felf64 123i.asm
ld -o 123i 123i.o

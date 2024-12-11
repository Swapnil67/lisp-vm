#!/bin/sh

set -xe

./build.sh

./basm2amd64 ./examples/halt.basm > halt.asm
nasm -felf64 halt.asm
ld -o halt halt.o

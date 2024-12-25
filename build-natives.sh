#!/bin/sh

set -xe

nasm -felf64 -g native_test.asm -o ./build/native_test.o
ld -o ./build/native_test ./build/native_test.o

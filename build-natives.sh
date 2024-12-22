#!/bin/sh

set -xe

nasm -felf64 -g native_test.asm -o native_test.o
ld -o native_test native_test.o

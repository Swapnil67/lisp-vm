
#!/bin/sh

set -xe

./build.sh

./build/bin/basm2amd64 ./examples/fib.basm > ./build/examples/fib.asm
nasm -felf64 -F dwarf -g ./build/examples/fib.asm -o ./build/examples/fib.o
ld -o ./build/examples/fib.exe ./build/examples/fib.o

./build/bin/basm2amd64 ./examples/123i.basm > ./build/examples/123i.asm
nasm -felf64 -F dwarf -g ./build/examples/123i.asm -o ./build/examples/123i.o
ld -o ./build/examples/123i.exe ./build/examples/123i.o

./build/bin/basm2amd64 ./examples/ret.basm > ./build/examples/ret.asm
nasm -felf64 -F dwarf -g ./build/examples/ret.asm -o ./build/examples/ret.o
ld -o ./build/examples/ret.exe ./build/examples/ret.o

# call.basm -> call.asm
./build/bin/basm2amd64 ./examples/call.basm > ./build/examples/call.asm
nasm -felf64 -F dwarf -g ./build/examples/call.asm -o ./build/examples/call.o
ld -o ./build/examples/call.exe ./build/examples/call.o

# hello.basm -> hello.asm
./build/bin/basm2amd64 ./examples/hello.basm > ./build/examples/hello.asm
nasm -felf64 -F dwarf -g ./build/examples/hello.asm -o ./build/examples/hello.o
ld -o ./build/examples/hello.exe ./build/examples/hello.o

# read.basm -> read.asm
./build/bin/basm2amd64 ./examples/read.basm > ./build/examples/read.asm
nasm -felf64 -F dwarf -g ./build/examples/read.asm -o ./build/examples/read.o
ld -o ./build/examples/read.exe ./build/examples/read.o

# write.basm -> write.asm
./build/bin/basm2amd64 ./examples/write.basm > ./build/examples/write.asm
nasm -felf64 -F dwarf -g ./build/examples/write.asm -o ./build/examples/write.o
ld -o ./build/examples/write.exe ./build/examples/write.o


# native_write.basm -> native_write.asm
./build/bin/basm2amd64 ./examples/native_write.basm > ./build/examples/native_write.asm
nasm -felf64 -F dwarf -g ./build/examples/native_write.asm -o ./build/examples/native_write.o
ld -o ./build/examples/native_write.exe ./build/examples/native_write.o

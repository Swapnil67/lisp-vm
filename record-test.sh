#!/bin/sh

set -xe

./build.sh

rm -r test/
mkdir -p test/examples

for example in `find examples/ -name \*.basm | sed "s/\.basm//"`; do
    if [ "$example" != "examples/ret" ] &&  [ "$example" != "examples/alloc" ]
    then
	./build/bin/bmr -p  "build/$example.bm" -ao "./test/$example.expected.out"
    else 
	echo $example
    fi
done


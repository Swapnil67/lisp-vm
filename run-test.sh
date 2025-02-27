#!/bin/sh

set -xe

./build.sh

for example in `find examples/ -name \*.basm | sed "s/\.basm//"`; do
    if [ "$example" != "examples/ret" ] &&  [ "$example" != "examples/alloc" ]
    then
	./build/bin/bmr -p  "build/$example.bm" -eo "./test/$example.expected.out"
    else 
	echo $example
    fi
done


#!/bin/bash

sources=$(ls *.brash)

function runTest() {
	src="$1"
	actual=$(echo $src | sed 's/.brash$/.bro/')
	expected=$(echo $src | sed 's/.brash$/.expected/')

	./brash -c $src -o $actual -s

	if [[ -e $expected ]]; then
		cmp -s $actual $expected && printf "%-10s %s\n" "[PASS]" $src || printf "%-10s %s\n" "[FAIL]" $src
	else
		printf "%-10s %s\n" "[NEW]" $src
		cp $actual $expected
	fi
}

for x in $sources; do
	runTest $x
done

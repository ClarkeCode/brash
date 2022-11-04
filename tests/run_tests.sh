#!/bin/bash
COMPDIR="compilation/"
TERPDIR="interpretation/"
LEXRDIR="tokenization/"
sources=$(ls *.brash)

function compTest() {
	src="$1"
	actual=$COMPDIR$(echo $src | sed 's/.brash$/.bro/')
	expected=$COMPDIR$(echo $src | sed 's/.brash$/.expected/')

	./brash -c $src -o $actual -s

	if [[ -e $expected ]]; then
		cmp -s $actual $expected && printf "%-10s %s\n" "[PASS]" $src || printf "%-10s %s\n" "[FAIL]" $src
	else
		printf "%-10s %s\n" "[NEW]" $src
		cp $actual $expected
	fi
}

function outputTest() {
	src="$1"
	dir="$2"
	actual=$dir$(echo $src | sed 's/.brash$/.output/')
	expected=$dir$(echo $src | sed 's/.brash$/.expected/')

	./brash $src -s > $actual

	if [[ -e $expected ]]; then
		cmp -s $actual $expected && printf "%-10s %s\n" "[PASS]" $src || printf "%-10s %s\n" "[FAIL]" $src
	else
		printf "%-10s %s\n" "[NEW]" $src
		cp $actual $expected
	fi
}

echo "=== TOKENIZATION TESTS ==="
for x in $sources; do
	outputTest $x $LEXRDIR
done
echo

echo "=== COMPILATION TESTS ==="
for x in $sources; do
	compTest $x
done
echo

echo "=== INTERPRETATION TESTS ==="
for x in $sources; do
	outputTest $x $TERPDIR
done

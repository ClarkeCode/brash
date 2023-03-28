#!/bin/bash
COMPDIR="compilation/"
TERPDIR="interpretation/"
LEXRDIR="tokenization/"
sources=$@

for x in $sources; do
	expected=$(echo $x | sed 's/.brash$/.expected/')
	rm -f $COMPDIR$expected $TERPDIR$expected $LEXRDIR$expected
done

#!/usr/bin/bash

function makeSwitch {
	local line=$(echo "$1" | sed 's/=.*,/,/g')
	local enum_vals=$(echo "$line" | grep -Eo "\{.*\}" | sed 's/[\{\},]/ /g')
	local enum_name=$(echo "$line" | grep -Eo "}.*;" | sed 's/[ ;\}]//g')
#	echo "$line"
#	echo "$enum_vals"
#	echo "$enum_name"

	printf "char* getStr_%s(%s lookup) {\n" "$enum_name" "$enum_name"
	printf "\tswitch(lookup) {\n"
	for val in $enum_vals; do
		printf "\t\tcase %-30s: return \"%s\";\n" "$val" "$val"
	done
	printf "\t\tdefault: return \"LOOKUP_UNKNOWN_VALUE\";\n";
	printf "\t}\n}\n"
}

function processFile {
	cat "$1" | tr -d '\n' | tr -d '\t' | sed 's/;/;\n/g' | grep "typedef enum.*;" | while read line; do
		makeSwitch "$line"
	done
}



if [ $# -eq 0 ]; then
	    echo "Usage: $0 [FILE]..."
	    exit 1
fi

echo "//Generated file"
for file in "$@"
do
	grep -q "typedef enum" $file && printf "#include \"%s\"\n" $file
	processFile $file
done

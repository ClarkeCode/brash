#!/bin/bash
#Generates a dependancy map for .c and .h files in a graphviz-compatable format 
if [ $# -eq 0 ]; then
	echo "Usage: $0 [FILES]..."
	exit 1
fi

echo "//Generated file"
echo "digraph G {"
printf "\t\"main.c\"[shape=diamond]\n"
grep -o "#include \".*\"" "$@" | sed 's/\(.*\?\):.*\(".*"\)/\t\2->"\1"/' | sed 's/\(".*\.h"\)->/\1[shape=rectangle]\n\t\0/'
echo "}"

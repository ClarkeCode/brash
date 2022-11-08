#!/bin/bash

#Generates a header according to the functions declared in the specified .c file
SOURCE_FILE="$1"
GENERATED_FILE=$(echo "$SOURCE_FILE" | sed 's/.c$/.h/')
HEADER_GUARD_LABEL="$2"

[[ -z "$SOURCE_FILE" ]] && echo "$0 ERROR: Must specify target .c file" && exit
[[ -z "$HEADER_GUARD_LABEL" ]] && echo "$0 ERROR: Must specify header guard label" && exit

echo "//Generated file"
echo "#ifndef $HEADER_GUARD_LABEL"
echo "#define $HEADER_GUARD_LABEL"
grep -E "^#include" "$SOURCE_FILE" | grep -Ev "$GENERATED_FILE"
grep -E "(.* {)" "$SOURCE_FILE" | grep -Ev "if|else|while|for|switch|struct" | sed 's/ {/;/'
echo "#endif"

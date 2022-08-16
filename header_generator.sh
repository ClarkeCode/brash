#!/bin/bash

#Generates a header according to the functions declared in the specified .c file
SOURCE_FILE="$1"
GENERATED_FILE=$(echo "$SOURCE_FILE" | sed 's/.c$/.h/')
HEADER_GUARD_LABEL="$2"

[[ -z "$SOURCE_FILE" ]] && echo "$0 ERROR: Must specify target .c file" && exit
[[ -z "$HEADER_GUARD_LABEL" ]] && echo "$0 ERROR: Must specify header guard label" && exit

echo "//Generated-file" > "$GENERATED_FILE"
echo "#ifndef $HEADER_GUARD_LABEL" >> "$GENERATED_FILE"
echo "#define $HEADER_GUARD_LABEL" >> "$GENERATED_FILE"
grep -E "^#include" "$SOURCE_FILE" | grep -Ev "$GENERATED_FILE" >> "$GENERATED_FILE"
grep -E "(.* {)" "$SOURCE_FILE" | grep -Ev "if|else|while|for|switch" | sed 's/ {/;/' >> "$GENERATED_FILE"
echo "#endif" >> "$GENERATED_FILE"

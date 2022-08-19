#!/bin/bash
FILE="$1"

echo "#include \"enum_lookups.h\""
echo "#include \"structs.h\""
awk '/typedef enum/,/;/{ print }' $FILE | tail -1 | sed 's/[} ;]//g' | \awk '{printf("char* getStr_%s(%s lookup) {\n\tswitch(lookup) {\n", $0, $0)}'

awk '/typedef enum/,/;/{ print }' $FILE | sed '1d;$d;s/,//;s/\t//;/^$/d' | awk '{ printf("\t\tcase %-25s: return \"%s\";\n", $0, $0) }'

echo "
		default: return \"LOOKUP_UNKNOWN_VALUE\";
	}
}" 

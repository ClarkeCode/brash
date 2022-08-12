#!/bin/bash
awk '/typedef enum/,/;/{ print }' main.c | tail -1 | sed 's/[} ;]//g' | \awk '{printf("char* getStr_%s(%s lookup) {\n\tswitch(lookup) {\n", $0, $0)}'

awk '/typedef enum/,/;/{ print }' main.c | sed '1d;$d;s/,//;s/\t//;/^$/d' | awk '{ printf("\t\tcase %-20s: return \"%s\";\n", $0, $0) }'

echo "
		default: return \"LOOKUP_UNKNOWN_VALUE\";
	}
}" 

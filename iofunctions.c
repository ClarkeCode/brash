#include <stdlib.h>
#include <stdio.h>
#include "iofunctions.h"

char* readFile(char* filePath) {
	FILE* sourceFile = fopen(filePath, "rb");

	fseek(sourceFile, 0, SEEK_END);
	size_t fsize = ftell(sourceFile);
	rewind(sourceFile);

	char* source = (char*) malloc(fsize + 1);
	size_t nbytes = fread(source, sizeof(char), fsize, sourceFile);
	source[nbytes] = '\0';

	fclose(sourceFile);
	return source;
}

size_t lenFile(char* filePath) {
	FILE* sourceFile = fopen(filePath, "rb");

	fseek(sourceFile, 0, SEEK_END);
	size_t fsize = ftell(sourceFile);
	rewind(sourceFile);

	fclose(sourceFile);
	return fsize;
}

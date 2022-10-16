#include <stdio.h>
#include <stdlib.h> //size_t
#include <stdbool.h> //bool
#include <string.h>

#include "scline.h"
#include "enum_lookups.h"
#include "lexer.h"
#include "compiler.h"

#include "debugging.h"

int main(/*int argc, char* argv[]*/) {
	setDebugFlags(0);

	char* program = "1 + 2";

	Chunk chunk;
	initChunk(&chunk);
	bool success = compile(program, &chunk);
	printf((success ? "Compilation successful\n" : "Compilation failure\n"));
	disassembleChunk(&chunk, "test chunk");
	freeChunk(&chunk);
	return 0;
}

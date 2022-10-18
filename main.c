#include <stdio.h>
#include <stdlib.h> //size_t
#include <stdbool.h> //bool
#include <string.h>

#include "scline.h"
#include "enum_lookups.h"
#include "compiler.h"
#include "virtualmachine.h"

#include "debugging.h"

int main(/*int argc, char* argv[]*/) {
	setDebugFlags(VM_READING_INSTRUCTIONS);

	char* program = "1 + 2";

	Chunk chunk;
	initChunk(&chunk);
	bool success = compile(program, &chunk);
	printf((success ? "Compilation successful\n" : "Compilation failure\n"));
	disassembleChunk(&chunk, "test chunk");
	freeChunk(&chunk);

	//InterpretResult result = interpret(program);
	//printf("%s\n", getStr_InterpretResult(result));
	return 0;
}

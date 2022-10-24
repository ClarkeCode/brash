#include <stdio.h>
#include <stdlib.h> //size_t
#include <stdbool.h> //bool
#include <string.h>

#include "scline.h"
#include "enum_lookups.h"
#include "compiler.h"
#include "virtualmachine.h"

#include "debugging.h"

int main(int argc, char* argv[]) {
	argc--; argv++;
	setDebugFlags(0);//LEX_TOKEN_PRODUCTION | COM_BYTE_EMISSION | VM_READING_INSTRUCTIONS | VM_STACK_TRACE | COM_CHUNK_DISASSEMBLY);

	bool doRepl = argc == 0;

	initVM();
	if (doRepl) {
		CmdLine* cline = make_cmdline("brash> ", 8);
		do {
			char* program = grabInputLine(cline);
			printf("\n");

			if (strcmp(program, "quit") == 0) {
				free(program);
				break;
			}
			InterpretResult result = interpret(program);
			printf("%s\n", getStr_InterpretResult(result));
			free(program);
		} while (true);
		free_cmdline(cline);
		freeObjects();
	}
	else {
		char* program = argv[0];// = "var name = \"Greg\"+\" Jones\"\n name + \" Jr.\"";
		InterpretResult result = interpret(program);
		freeObjects();
		printf("%s\n", getStr_InterpretResult(result));
	}

	return 0;
}

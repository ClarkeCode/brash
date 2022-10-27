#include <stdio.h>
#include <stdlib.h> //size_t
#include <stdbool.h> //bool
#include <string.h>

#include "scline.h"
#include "enum_lookups.h"
#include "compiler.h"
#include "virtualmachine.h"

#include "iofunctions.h"
#include "debugging.h"

typedef struct {
	bool modeRepl;
	bool modeInterpret;
	bool modeCompilation;
	bool modeDisassembly;

	bool behaviourSilent;

	char* infile;
	char* outfile;
} ProgramInput;

int main(int argc, char* argv[]) {
	ProgramInput pinput = {0};
	{
		argc--; argv++;
		if (argc == 0) pinput.modeRepl = true;
		while (argc > 0) {
			if (argv[0] && strcmp(argv[0], "-c") == 0) {
				argc--; argv++;
				if (argc == 0) break; //no file specified
				pinput.modeCompilation = true;
				pinput.infile = argv[0];
				argc--; argv++;
			}
			if (argv[0] && strcmp(argv[0], "-o") == 0) {
				argc--; argv++;
				if (argc == 0) break; //no file specified
				pinput.outfile = argv[0];
				argc--; argv++;
			}
			if (argv[0] && strcmp(argv[0], "-D") == 0) {
				argc--; argv++;
				if (argc == 0) break; //no file specified
				pinput.modeDisassembly = true;
				pinput.infile = argv[0];
				argc--; argv++;
			}
			if (argv[0] && strcmp(argv[0], "-s") == 0) {
				argc--; argv++;
				pinput.behaviourSilent = true;
			}
			else {
				pinput.modeInterpret = true;
				pinput.infile = argv[0];
				argc--; argv++;
			}
		}
	}

	if (pinput.behaviourSilent)
		setDebugFlags(0);
	else
		setDebugFlags(LEX_TOKEN_PRODUCTION | COM_BYTE_EMISSION | VM_READING_INSTRUCTIONS | VM_STACK_TRACE | COM_CHUNK_DISASSEMBLY);

	if (pinput.modeCompilation) { //Compile
		char* srcfile = pinput.infile;
		char* source = readFile(srcfile);

		char* outfile = (pinput.outfile ? pinput.outfile : "a.bro");

		Chunk chunk;
		initChunk(&chunk);
		bool compSuccess = compile(source, &chunk);
		serializeChunk(&chunk, outfile);
		freeChunk(&chunk);
		free(source);
		return (compSuccess ? EXIT_SUCCESS : EXIT_FAILURE);
	}

	if (pinput.modeDisassembly) { //Disassemble
		char* content = readFile(pinput.infile);

		Chunk chunk;
		initChunk(&chunk);
		chunk.code = (byte_t*) content;
		chunk.size = lenFile(pinput.infile);
		disassembleChunk(&chunk, "");
		free(content);
		return EXIT_SUCCESS;
	}

	initVM((pinput.modeRepl ? VM_MODE_INTERACTIVE : VM_MODE_STATIC));
	if (pinput.modeRepl) {
		CmdLine* cline = make_cmdline("brash> ", 8);
		do {
			char* program = grabInputLine(cline);
			printf("\n");

			if (strcmp(program, "quit") == 0) {
				free(program);
				break;
			}
			/*InterpretResult result = */interpret(program);
			//printf("%s\n", getStr_InterpretResult(result));
			free(program);
		} while (true);
		free_cmdline(cline);
		freeObjects();
	}
	else if (pinput.modeInterpret) {
		char* srcfile = pinput.infile;
		char* source = readFile(srcfile);
		InterpretResult result = interpret(source);
		free(source);
		freeObjects();
		printf("%s\n", getStr_InterpretResult(result));
	}

	return 0;
}

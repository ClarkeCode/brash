#include "virtualmachine.h"
#include "compiler.h"

VM vm;

InterpretResult run() {
	//TODO: implement
	return INTERPRET_OK;
}

bool typecheck(Chunk* chunk) {
	if (chunk->size == 0) return true; //This line just prevents warnings until implementation
	//TODO: implement type checking
	return true;
}

InterpretResult interpret(const char* source) {
	Chunk chunk;
	initChunk(&chunk);

	if (!compile(source, &chunk)) {
		freeChunk(&chunk);
		return INTERPRET_COMPILE_ERROR;
	}

	if (!typecheck(&chunk)) {
		freeChunk(&chunk);
		return INTERPRET_TYPECHECK_ERROR;
	}
	
	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;

	InterpretResult result = run();
	freeChunk(&chunk);
	return result;
}

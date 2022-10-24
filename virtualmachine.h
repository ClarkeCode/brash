#ifndef BRASH_VM
#define BRASH_VM
#include "chunk.h"
#include "variablelookup.h"

#define STACK_MAX 512
typedef struct {
	Chunk* chunk;
	byte_t* ip;
	Value stack[STACK_MAX];
	Value* stackTop;

	VariableLookup* lookup;

	Object* objects;
	Object* mostRecentObject;
	size_t allocatedBytes;
} VM;
#undef STACK_MAX

extern VM vm;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_TYPECHECK_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
InterpretResult interpret(const char* source);
void freeObjects();


void push(Value value);
Value pop();
#endif

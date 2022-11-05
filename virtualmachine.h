#ifndef BRASH_VM
#define BRASH_VM
#include "chunk.h"
#include "variablelookup.h"

typedef enum {
	VM_MODE_STATIC,
	VM_MODE_INTERACTIVE,
} VM_Mode;

#define STACK_MAX 512
typedef struct {
	VM_Mode mode;
	Chunk* chunk;
	byte_t* ip;
	Value stack[STACK_MAX];
	Value* stackTop;

	VariableLookup* variableTables[STACK_MAX]; //TODO: make dynamic
	size_t scopeLevel;

	Object* objects;
	Object* mostRecentObject;
	size_t allocatedBytes;
} VM;
//#undef STACK_MAX

extern VM vm;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_TYPECHECK_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM(VM_Mode vmMode);
InterpretResult interpret(const char* source);
void freeVM();


void push(Value value);
Value pop();
#endif

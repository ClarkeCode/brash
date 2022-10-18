#include <stdio.h>
#include <math.h> //fmod
#include "virtualmachine.h"
#include "compiler.h"
#include "debugging.h"
#include "enum_lookups.h"

VM vm;

void resetStack() { vm.stackTop = vm.stack; }
void initVM() { resetStack(); }

void push(Value value){
	*vm.stackTop = value;
	vm.stackTop++;
}
Value pop(){
	vm.stackTop--;
	return *vm.stackTop;
}

InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants[READ_BYTE()])
	while (true) {
		byte_t instruction = READ_BYTE();
		printDebug(stdout, VM_READING_INSTRUCTIONS, "[VM] Reading instruction '%s'\n", getStr_OpCode(instruction));
		switch (instruction) {
			case OP_NUMBER: {
				Value constant = READ_CONSTANT();
				printValue(stdout, constant);
				printf("\n");
				push(constant);
				} break;

			//Arithmetic instructions
			case OP_NEGATE: {
				Value val = pop();
				val.as.number = -(val.as.number);
				push(val);
				} break;
			case OP_ADD: {
				Value b = pop();
				Value a = pop();
				Value result = { VAL_NUMBER, {.number=(a.as.number + b.as.number)} };
				push(result);
				} break;
			case OP_SUBTRACT: {
				Value b = pop();
				Value a = pop();
				Value result = { VAL_NUMBER, {.number=(a.as.number - b.as.number)} };
				push(result);
				} break;
			case OP_MULTIPLY: {
				Value b = pop();
				Value a = pop();
				Value result = { VAL_NUMBER, {.number=(a.as.number * b.as.number)} };
				push(result);
				} break;
			case OP_DIVIDE: {
				Value b = pop();
				Value a = pop();
				Value result = { VAL_NUMBER, {.number=(a.as.number / b.as.number)} };
				push(result);
				} break;
			case OP_MODULO: {
				Value b = pop();
				Value a = pop();
				Value result = { VAL_NUMBER, {.number=fmod(a.as.number, b.as.number)} };
				push(result);
				} break;

			case OP_RETURN: {
				return INTERPRET_OK;
				} break;
		}
	}
	//TODO: implement
	return INTERPRET_OK;

#undef READ_BYTE
#undef READ_CONSTANT
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
	disassembleChunk(&chunk, "test chunk");

	InterpretResult result = run();
	freeChunk(&chunk);
	return result;
}

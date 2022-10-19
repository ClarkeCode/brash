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
	while (true) {
		byte_t instruction = READ_BYTE();
		printDebug(stdout, VM_READING_INSTRUCTIONS, "[VM] Reading instruction '%s'", getStr_OpCode(instruction));
		switch (instruction) {
			case OP_NUMBER: {
					Value constant = { VAL_NUMBER, {.number=(readDoubleFromBytes(vm.ip))} };
					vm.ip += 8;

					printDebug(stdout, VM_READING_INSTRUCTIONS, " <%s : %f>", getStr_value_t(constant.type), constant.as.number);
					push(constant);
				} break;
			case OP_TRUE: {
					Value val = { VAL_BOOLEAN, {.boolean=(true)} };
					push(val);
				} break;
			case OP_FALSE: {
					Value val = { VAL_BOOLEAN, {.boolean=(false)} };
					push(val);
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

			//Comparison instructions
			case OP_EQUALS: {
					Value b = pop();
					Value a = pop();
					Value val = { VAL_BOOLEAN, {.boolean=(valueEquality(a, b))} };
					push(val);
				} break;
			case OP_GREATER: {
					Value b = pop();
					Value a = pop();
					Value val = { VAL_BOOLEAN, {.boolean=(a.as.number > b.as.number)} };
					push(val);
				} break;
			case OP_LESSER: {
					Value b = pop();
					Value a = pop();
					Value val = { VAL_BOOLEAN, {.boolean=(a.as.number < b.as.number)} };
					push(val);
				} break;

			//Logical instructions
			case OP_NOT: {
					Value val = pop();
					val.as.boolean = !(val.as.boolean);
					push(val);
				} break;
			case OP_AND: {
					Value b = pop();
					Value a = pop();
					Value result = { VAL_BOOLEAN, {.number=(a.as.boolean && b.as.boolean)} };
					push(result);
				} break;
			case OP_OR: {
					Value b = pop();
					Value a = pop();
					Value result = { VAL_BOOLEAN, {.number=(a.as.boolean || b.as.boolean)} };
					push(result);
				} break;
			case OP_XOR: {
					Value b = pop();
					Value a = pop();
					Value result = { VAL_BOOLEAN, {.number=(a.as.boolean ^ b.as.boolean)} };
					push(result);
				} break;

			case OP_RETURN: {
					printDebug(stdout, VM_READING_INSTRUCTIONS, "\n");
					return INTERPRET_OK;
				} break;
		}
		printDebug(stdout, VM_READING_INSTRUCTIONS, "\n");
	}
	//TODO: implement
	return INTERPRET_OK;

#undef READ_BYTE
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
	
	initVM();
	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;
	disassembleChunk(&chunk, "test chunk");

	InterpretResult result = run();
	if (hasDebugFlag(VM_STACK_TRACE) && vm.stackTop != vm.stack) {
		printDebug(stdout, VM_STACK_TRACE, "[VM] Finished with %d items on the value stack:\n", vm.stackTop - vm.stack);
		while (vm.stackTop-- != vm.stack) {
			if (vm.stackTop->type == VAL_NUMBER)
				printDebug(stdout, VM_STACK_TRACE, "\t<%s : %f>\n", getStr_value_t(vm.stackTop->type), vm.stackTop->as.number);
			else
				printDebug(stdout, VM_STACK_TRACE, "\t<%s : %s>\n", getStr_value_t(vm.stackTop->type), (vm.stackTop->as.boolean ? "true" : "false"));
		}
	}
	freeChunk(&chunk);
	return result;
}

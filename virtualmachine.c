#include <stdio.h>
#include <math.h> //fmod
#include "virtualmachine.h"
#include "compiler.h"
#include "debugging.h"
#include "enum_lookups.h"

VM vm;

void resetStack() { vm.stackTop = vm.stack; }
void initVM(VM_Mode vmMode) {
	vm.mode = vmMode;
	vm.objects = NULL;
	vm.mostRecentObject = NULL;
	resetStack();

	vm.scopeLevel = 0;
	for (size_t x = 0; x < STACK_MAX; x++) {
		vm.variableTables[x] = make_variable_lookup();
	}
}

void addObject(Object* object) {
	if (vm.objects == NULL) {
		vm.objects = object;
	}
	else {
		vm.mostRecentObject->next = object;
	}
	vm.mostRecentObject = object;
}

void freeObjects() {
	Object* object = vm.objects;
	while (object != NULL) {
		Object* next = object->next;
		freeObject(object);
		object = next;
	}
}
void freeVM() {
	freeObjects();
	for (size_t x = 0; x < STACK_MAX; x++) {
		free_variable_lookup(vm.variableTables[x]);
	}
}

void push(Value value){
	*vm.stackTop = value;
	vm.stackTop++;
}
Value pop(){
	vm.stackTop--;
	return *vm.stackTop;
}
VariableLookup* _variableLookupAtDepth(size_t depth) { return vm.variableTables[depth]; }
VariableLookup* _currentVariableLookup() { return _variableLookupAtDepth(vm.scopeLevel); }

#include <stdlib.h>
#include <string.h>
InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
	while (true) {
		OpCode instruction = (OpCode) READ_BYTE(); //cast from byte_t
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
			case OP_STRING: {
					//TODO:
					ObjectString* ostring = makeString((char*)vm.ip);
					addObject((Object*) ostring);
					Value valstring = { VAL_OBJECT, {.object=((Object*)ostring)} };
					printDebug(stdout, VM_READING_INSTRUCTIONS, " <%s : %s>", getStr_obj_t(valstring.as.object->type), ostring->cstr);
					vm.ip += ostring->len + 1;
					push(valstring);
				} break;

			case OP_ENTER_SCOPE: {
					vm.scopeLevel++;
				} break;
			case OP_EXIT_SCOPE: {
					if (vm.scopeLevel == 0) {
						fprintf(stderr, "Error: cannot exit top-level scope");
						exit(EXIT_FAILURE);
					}
					free_variable_lookup(_currentVariableLookup());
					vm.variableTables[vm.scopeLevel] = make_variable_lookup();
					vm.scopeLevel--;
				} break;

			case OP_LOOP: {
					//Note: jump distances are a relative jump from the jump instruction
					int16_t jumpDistance = readInt16FromBytes(vm.ip);
					vm.ip -= (jumpDistance + 1);
				} break;
			case OP_JUMP: {
					//Note: jump distances are a relative jump from the jump instruction
					int16_t jumpDistance = readInt16FromBytes(vm.ip);
					vm.ip += jumpDistance - 1;
				} break;
			case OP_JUMP_IF_FALSE: {
					Value val = pop();
					if (!val.as.boolean) {
						//Note: jump distances are a relative jump from the jump instruction
						int16_t jumpDistance = readInt16FromBytes(vm.ip);
						vm.ip += jumpDistance - 1;
					}
					else
						vm.ip += 2;
				} break;
			case OP_DEF_VARIABLE: {
					char* variableName = (char*) vm.ip;
					printDebug(stdout, VM_READING_INSTRUCTIONS, " '%s'", variableName);
					Value val = { VAL_NOT_VALUE, {0} };
					lookup_add(_currentVariableLookup(), variableName, val);
					vm.ip += strlen(variableName) + 1;
				} break;
			case OP_SET_VARIABLE: {
					char* variableName = (char*) vm.ip;
					Value val = pop();
					printDebug(stdout, VM_READING_INSTRUCTIONS, " '%s'", variableName);

					VariableLookup* lookup = NULL;

					//If we are assigning to a variable which was declared before the current scope, overwrite that value; do not make a new local variable
					if (vm.scopeLevel > 0) {
						size_t searchLevel = vm.scopeLevel - 1;
						while (lookup == NULL) {
							if (lookup_has(_variableLookupAtDepth(searchLevel), variableName)) {
								lookup = _variableLookupAtDepth(searchLevel);
							}

							if (searchLevel == 0 && lookup == NULL) {
								lookup = _currentVariableLookup();
							}
							searchLevel--;
						}
					}
					if (!lookup) lookup = _currentVariableLookup();

					lookup_add(lookup, variableName, val);
					vm.ip += strlen(variableName) + 1;
				} break;

			case OP_GET_VARIABLE: {
					char* variableName = (char*) vm.ip;
					printDebug(stdout, VM_READING_INSTRUCTIONS, " '%s'", variableName);

					//Search to see if the variable is defined, starting from the current scope -> top level scope
					size_t searchLevel = vm.scopeLevel;
					bool found = false;
					while (!found) {
						found = lookup_has(vm.variableTables[searchLevel], variableName);
						if (!found && vm.scopeLevel == 0) {
							fprintf(stderr, "ERR: Variable is not defined.\n");
							return INTERPRET_RUNTIME_ERROR;
						}
						else if (!found) {
							searchLevel--;
						}
						else {
							push(lookup_get(vm.variableTables[searchLevel], variableName));
							break;
						}
					}
					vm.ip += strlen(variableName) + 1;
				} break;


#define VAL_AS_STRING(val) (((ObjectString*) val.as.object)->cstr)
			case OP_POP: {
					Value val = pop();
					if (vm.mode == VM_MODE_INTERACTIVE) {
						if (val.type == VAL_NUMBER) fprintf(stdout, "%f\n", val.as.number);
						if (val.type == VAL_BOOLEAN) fprintf(stdout, "%s\n", (val.as.boolean ? "true" : "false"));
						if (val.type == VAL_OBJECT) {
							if (val.as.object->type == OBJ_STRING) {
								fprintf(stdout, "%s\n", VAL_AS_STRING(val));
							}
						}
					}
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

					if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
						Value result = { VAL_NUMBER, {.number=(a.as.number + b.as.number)} };
						push(result);
					}
					else if (a.type == VAL_OBJECT && a.as.object->type == OBJ_STRING &&
							b.type == VAL_OBJECT && b.as.object->type == OBJ_STRING) {
						ObjectString* ap = (ObjectString*)a.as.object;
						ObjectString* bp = (ObjectString*)b.as.object;
						char* concatenatedStr = calloc(sizeof(char), ap->len + bp->len + 1);
						strcpy(concatenatedStr, ap->cstr);
						strcpy(concatenatedStr + ap->len, bp->cstr);
						ObjectString* ostring = makeString(concatenatedStr);
						if (concatenatedStr) free(concatenatedStr);
						addObject((Object*) ostring);
						Value valstring = { VAL_OBJECT, {.object=((Object*)ostring)} };
						push(valstring);
					}
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
					Value result = { VAL_BOOLEAN, {.boolean=(a.as.boolean && b.as.boolean)} };
					push(result);
				} break;
			case OP_OR: {
					Value b = pop();
					Value a = pop();
					Value result = { VAL_BOOLEAN, {.boolean=(a.as.boolean || b.as.boolean)} };
					push(result);
				} break;
			case OP_XOR: {
					Value b = pop();
					Value a = pop();
					Value result = { VAL_BOOLEAN, {.boolean=(a.as.boolean ^ b.as.boolean)} };
					push(result);
				} break;

			case OP_PRINT: {
					Value val = pop();
					if (val.type == VAL_BOOLEAN)
						fprintf(stdout, (val.as.boolean ? "true" : "false"));
					else if (val.type == VAL_NUMBER)
						fprintf(stdout, "%lf", val.as.number);
					else if (val.type == VAL_OBJECT) {
						if (vm.stackTop->as.object->type == OBJ_STRING)
							fprintf(stdout, "%s", ((ObjectString*)vm.stackTop->as.object)->cstr);
					}
					fprintf(stdout, "\n");
				} break;

			case OP_RETURN: {
					printDebug(stdout, VM_READING_INSTRUCTIONS, "\n");
					return INTERPRET_OK;
				} break;
			default:
				fprintf(stderr, "Unknown OpCode: '%d'\n", instruction);
				break;
		}
		printDebug(stdout, VM_READING_INSTRUCTIONS, "\n");
	}

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
	
	//initVM();
	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;
	if (hasDebugFlag(COM_CHUNK_DISASSEMBLY)) {
		disassembleChunk(&chunk, "test chunk");
	}

	InterpretResult result = run();
	if (hasDebugFlag(VM_STACK_TRACE) && vm.stackTop > vm.stack) {
		printDebug(stdout, VM_STACK_TRACE, "[VM] Finished with %d items on the value stack:\n", vm.stackTop - vm.stack);
		while (vm.stackTop-- != vm.stack) {
			if (vm.stackTop->type == VAL_NUMBER)
				printDebug(stdout, VM_STACK_TRACE, "\t<%s : %f>\n", getStr_value_t(vm.stackTop->type), vm.stackTop->as.number);
			else if (vm.stackTop->type == VAL_BOOLEAN)
				printDebug(stdout, VM_STACK_TRACE, "\t<%s : %s>\n", getStr_value_t(vm.stackTop->type), (vm.stackTop->as.boolean ? "true" : "false"));
			else if (vm.stackTop->type == VAL_OBJECT) {
				printDebug(stdout, VM_STACK_TRACE, "\t[%s : '%s']\n", getStr_obj_t(vm.stackTop->as.object->type), ((ObjectString*)vm.stackTop->as.object)->cstr);
			}
			else
				printDebug(stdout, VM_STACK_TRACE, "\tUnprintable value\n");
		}
	}
	freeChunk(&chunk);
	return result;
}

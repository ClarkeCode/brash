#include "bytecodeconsumers.h"

#define AS_STRING (char*)
#define AS_TYPE (Types)
#include <string.h>
byte_t* decodeFunc(byte_t* bytecode, BrashFunc* func) {
	//if (*bytecode == OP_DEC_FUNCTION) bytecode++;
	
	int16_t bodyLength = readInt16FromBytes(bytecode);
	func->afterBody = bytecode + bodyLength;
	bytecode += 2;

	func->name = AS_STRING bytecode;
	bytecode += strlen(func->name) + 1;
	func->arity = (uint8_t) *bytecode++;
	for (size_t x = 0; x < func->arity; x++) {
		func->paramTypes[x] = AS_TYPE *bytecode++;
	}
	for (size_t x = 0; x < func->arity; x++) {
		func->paramNames[x] = AS_STRING bytecode;
		bytecode += strlen(func->paramNames[x]) + 1;
	}
	func->rarity = (uint8_t) *bytecode++;
	for (size_t x = 0; x < func->rarity; x++) {
		func->returnTypes[x] = AS_TYPE *bytecode++;
	}
	func->functionStart = bytecode;
	return func->afterBody;
}

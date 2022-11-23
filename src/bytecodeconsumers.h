#ifndef BRASH_BC_CONSUMERS
#define BRASH_BC_CONSUMERS

#include "bytes.h"
#include "typechecker.h"
typedef struct {
	char* name;
	uint8_t arity, rarity;
	Types paramTypes[256];
	char* paramNames[256];
	Types returnTypes[256];
	byte_t* functionStart;
	byte_t* afterBody;
} BrashFunc;

byte_t* decodeFunc(byte_t* bytecode, BrashFunc* func);

#endif

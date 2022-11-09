#ifndef BRASH_TYPECHECKER
#define BRASH_TYPECHECKER
#include "chunk.h"

typedef enum {
	TYPE_INVALID,
	TYPE_NUMBER,
	TYPE_BOOLEAN,
	TYPE_STRING,
	TYPE_USER_DEFINED,
} Types;

bool typecheck(Chunk* chunk);

#endif

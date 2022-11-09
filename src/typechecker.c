#include "typechecker.h"
#include <string.h>

typedef struct {
	Types* tstack;
	Types* stackTop;
	size_t capacity;
} TM;



bool typecheck(Chunk* chunk) {
	if (chunk->size == 0) return true;
	//TODO: implement type checking
	return true;
}

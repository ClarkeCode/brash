#ifndef BRASH_COMPILER
#define BRASH_COMPILER
#include <stdbool.h>
#include "lexer.h" //Token
#include "chunk.h"

typedef struct {
	Token current;
	Token previous;

	bool hadError;
	bool panicMode;
} Parser;

bool compile(const char* source, Chunk* chunk);
#endif

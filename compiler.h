#ifndef BRASH_COMPILER
#define BRASH_COMPILER
#include <stdbool.h>

#include "lexer.h" //StrView + Location + token_t
#include "chunk.h"

typedef struct {
	token_t type;
	StrView content;
	Location location;
} Token;

typedef struct {
	Token current;
	Token previous;

	bool hadError;
	bool panicMode;
} Parser;

bool compile(const char* source, Chunk* chunk);
#endif

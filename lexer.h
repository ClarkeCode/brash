#ifndef BRASH_LEXER
#define BRASH_LEXER
#include "enumerations.h"
typedef struct {
	char* original;
	size_t length;
} StrView;

char* unbox(StrView* strview);

typedef struct {
	char* file; //May be null when input is not from file; ie: REPL
	size_t line;
	size_t offset;
} Location;

void setLexer(char* filename, char* program);
token_t produceNextToken(StrView* content, Location* loc);
#endif

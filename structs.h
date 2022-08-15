#ifndef BRASH_STRUCTURES
#define BRASH_STRUCTURES
#include "enumerations.h"

typedef struct {
	token_t type;
	char* content;

	char*  origin_file;
	size_t origin_line;
	size_t origin_char;

} Token;


typedef struct {
	char* line;
	char* source_file;
	size_t line_offset;
	size_t char_offset;
} Lexer;

#endif

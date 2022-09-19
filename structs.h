#ifndef BRASH_STRUCTURES
#define BRASH_STRUCTURES
#include <stddef.h>
#include <stdbool.h>
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

typedef enum {
	TYPE_NOT_VALUE,
	TYPE_BOOLEAN,
	TYPE_NUMBER,
	TYPE_STRING
} value_t;

typedef struct {
	value_t type;
	union {
		bool boolean;
		double number;
		char* string;
	} as;
} Value;

typedef struct {
	Token* feed_tape;
	size_t tape_size;
	size_t tape_offset;
} Parser;

typedef struct ASTNode_ {
	Token* token;
	Value value;
	struct ASTNode_* parent;
	struct ASTNode_* left;
	struct ASTNode_* right;
} ASTNode;

typedef struct {
	char** _names;
	size_t _name_capacity;
	size_t _internal_size;

	Value* _values;
} VariableLookup;

typedef struct {
	VariableLookup* lookup;

	Value* _stack;
	size_t _top_index;
	size_t _max_index;
} Interpreter;
#endif

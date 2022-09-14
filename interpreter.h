#ifndef BRASH_INTERPRETER
#define BRASH_INTERPRETER
#include <stdio.h>
#include "structs.h"
#include "enumerations.h"
#include "parser_ast.h"
#include "variablelookup.h"

typedef struct {
	VariableLookup* lookup;

	Value* _stack;
	size_t _top_index;
	size_t _max_index;
} Interpreter;

void dump_value(FILE* fp, Value* val);
Interpreter* make_interpreter();
void free_interpreter(Interpreter* terp);
void dump_interpreter(FILE* fp, Interpreter* terp);
Value pop(Interpreter* terp, FILE* fp);
void push(Interpreter* terp, Value val, FILE* fp);
void interpret(Interpreter* terp, ASTNode* node, FILE* fp);
#endif

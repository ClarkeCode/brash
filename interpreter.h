#ifndef BRASH_INTERPRETER
#define BRASH_INTERPRETER
#include <stdio.h>
#include "structs.h"
#include "parser_ast.h"

Interpreter* make_interpreter();
void free_interpreter(Interpreter* terp);
Value pop(Interpreter* terp, FILE* fp);
void push(Interpreter* terp, Value val, FILE* fp);
void interpret(Interpreter* terp, ASTNode* node, FILE* fp);
#endif

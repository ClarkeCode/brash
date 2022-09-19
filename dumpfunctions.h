#ifndef BRASH_OUTPUT_FUNCS
#define BRASH_OUTPUT_FUNCS
#include <stdio.h>
#include <stdbool.h>
#include "structs.h"
void dump_lexer_tokens(FILE* outfile, Token* token_arr, size_t arr_sz);
void dump_value(FILE* fp, Value* val);
void dump_value_notype(FILE* fp, Value* val);
void dump_node(FILE* fp, ASTNode* ast);
void dump_tree(FILE* fp, ASTNode* rootNode);
void dump_variable_lookup(FILE* fp, VariableLookup* lookup);
void dump_interpreter(FILE* fp, Interpreter* terp);
#endif

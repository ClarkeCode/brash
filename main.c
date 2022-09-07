#include <stdio.h>
#include <stdlib.h> //size_t
#include <stdbool.h> //bool
#include <string.h>
#include <assert.h> //assert

#include "structs.h"
#include "enumerations.h"
#include "enum_lookups.h"
#include "lexer.h"
#include "parser_ast.h"

typedef struct {
	Value* _stack;
	size_t _top_index;
	size_t _max_index;
} Interpreter;

Interpreter* make_interpreter() {
	Interpreter* terp = (Interpreter*) calloc(1, sizeof(Interpreter));
	terp->_max_index = 256;
	terp->_stack = (Value*) calloc(terp->_max_index, sizeof(Value));
	return terp;
}
void free_interpreter(Interpreter* terp) {
	if (!terp) return;
	if (terp->_stack) free(terp->_stack);
	if (terp) free(terp);
}
void dump_interpreter(FILE* fp, Interpreter* terp) {
	for (size_t x = 0; x < terp->_top_index; x++) {
		if ((terp->_stack + x)->type == TYPE_NUMBER)
			fprintf(fp, "%3d %-10s %f\n", x, "NUMBER",  (terp->_stack + x)->as.number);
		else if ((terp->_stack + x)->type == TYPE_BOOLEAN)
			fprintf(fp, "%3d %-10s %s\n", x, "BOOLEAN", (terp->_stack + x)->as.boolean ? "true" : "false");
		else if ((terp->_stack + x)->type == TYPE_STRING)
			fprintf(fp, "%3d %-10s %s\n", x, "STRING",  (terp->_stack + x)->as.string);
		else
			fprintf(fp, "%3d %-10s\n", x, "NON-VALUE");
	}
}

Value pop(Interpreter* terp) {
	if (terp->_top_index - 1 == (size_t)-1) {fprintf(stderr, "ERR: Stack Underflow\n");} //Stack underflow
	return terp->_stack[--(terp->_top_index)];
}
void push(Interpreter* terp, Value val) {
	if (terp->_top_index + 1 >= terp->_max_index) {fprintf(stderr, "ERR: Stack Overflow\n");} //Stack overflow
	terp->_stack[(terp->_top_index)++] = val; //Copy
}

void interpret(Interpreter* terp, ASTNode* node) {
	if (!node) return;
	if (node->value.type != TYPE_NOT_VALUE) {
		push(terp, node->value);
	}

	interpret(terp, node->left);
	interpret(terp, node->right);
}

int main(int argc, char* argv[]) {

	char* program;
	if (argc > 1) { program = argv[1]; }
	else          { program = "2 + 3 * 4"; }
	
	Lexer* lex = make_lexer(program, "notfile", 0);

	size_t num_tok = 0;
	Token* tok = lexer_tokenize_all(lex, &num_tok);
	{
		fprintf(stdout, "=== LEXER ===\n");
		FILE* outfile = fopen("lexer.out", "wb");
		dump_lexer_tokens(stdout, tok, num_tok);
		fprintf(stdout, "Remaining in lexer: '%s'\n", lex->line + lex->char_offset);
		dump_lexer_tokens(outfile, tok, num_tok);
		fclose(outfile);
	}

	Parser* parser = make_parser(tok, num_tok);
	ASTNode* ast = parse_ast(parser, -1);
	{
		FILE* fp = fopen("ast.out", "w");
		fprintf(stdout, "\n=== PARSER ===\n");
		dump_tree(stdout, ast);
		fclose(fp);
	}

	Interpreter* terp = make_interpreter();
	{
		fprintf(stdout, "\n=== INTERPRETER ===\n");
		interpret(terp, ast);
		dump_interpreter(stdout, terp);
	}

	
	//Free allocated memory
	free_interpreter(terp);
	free(parser);
	free_astnode(ast);
	for (size_t x = 0; x < num_tok; x++) free((tok+x)->content);
	free(tok);
	free(lex);

	return 0;
}

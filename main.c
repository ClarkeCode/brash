#include <stdio.h>
#include <stdlib.h> //size_t
#include <stdbool.h> //bool
#include <string.h>

#include "structs.h"
#include "enumerations.h"
#include "enum_lookups.h"
#include "lexer.h"
#include "parser_ast.h"
#include "interpreter.h"
#include "variablelookup.h"

char* retrieveLine(FILE* stream) {
	char* line = malloc(100 * sizeof(char));
	size_t lsize = 0;
	size_t lcapacity = 100;

	if (line == NULL) return NULL; //Malloc failure

	while (true) {
		if (lsize == lcapacity) {
			lcapacity *= 2;
			char* nline = realloc(line, lcapacity);
			if (nline == NULL) { //Realloc failure
				free(line);
				return NULL;
			}
		}

		int c = fgetc(stream);
		if (c == EOF) break;
		if (c == '\n') break;
		line[lsize++] = c;
	}
	line[lsize] = '\0';
	return line;
}

void dump_value_notype(FILE* fp, Value* val) {
		if (val->type == TYPE_NUMBER)
			fprintf(fp, "%f", val->as.number);
		else if (val->type == TYPE_BOOLEAN)
			fprintf(fp, "%s", val->as.boolean ? "true" : "false");
		else if (val->type == TYPE_STRING)
			fprintf(fp, "%s", val->as.string);
}

int main(/*int argc, char* argv[]*/) {

	Interpreter* terp = make_interpreter();
	while (!lookup_has(terp->lookup, "quit")) {
		fprintf(stdout, "brash> ");
		char* line = retrieveLine(stdin);


		Lexer* lex = make_lexer(line, "notfile", 0);

		size_t num_tok = 0;
		Token* tok = lexer_tokenize_all(lex, &num_tok);
		{
			//fprintf(stdout, "=== LEXER ===\n");
			//dump_lexer_tokens(stdout, tok, num_tok);
			//fprintf(stdout, "Remaining in lexer: '%s'\n", lex->line + lex->char_offset);

			FILE* outfile = fopen("lexer.out", "wb");
			dump_lexer_tokens(outfile, tok, num_tok);
			fclose(outfile);
		}
		Parser* parser = make_parser(tok, num_tok);
		ASTNode* ast = parse_ast(parser, -1);
		{
			FILE* fp = fopen("ast.out", "wb");
			fprintf(fp, "\n=== PARSER ===\n");
			dump_tree(fp, ast);
			fclose(fp);

			//fprintf(stdout, "\n=== PARSER ===\n");
			//dump_tree(stdout, ast);
		}



		interpret(terp, ast, NULL);

		while (terp->_top_index != 0) {
			Value val = pop(terp, NULL);
			dump_value_notype(stdout, &val);
			printf("\n");
		}

		//Free allocated memory
		free(parser);
		free_astnode(ast);
		for (size_t x = 0; x < num_tok; x++) free((tok+x)->content);
		free(tok);
		free(lex);
		if (line) free(line);
	}

	fprintf(stdout, "\n=== INTERPRETER ===\n");
	fprintf(stdout, "Interpreter final state:\n");
	dump_interpreter(stdout, terp);

	free_interpreter(terp);
	return 0;
}

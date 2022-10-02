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

#include "dumpfunctions.h"
#include "scline.h"

int main(/*int argc, char* argv[]*/) {

	Interpreter* terp = make_interpreter();
	CmdLine* cmdline = make_cmdline("brash> ", 16);

	while (!lookup_has(terp->lookup, "quit")) {
		char* line = grabInputLine(cmdline);

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
			if (ast) dump_tree(fp, ast);
			fclose(fp);

			//fprintf(stdout, "\n=== PARSER ===\n");
			//dump_tree(stdout, ast);
		}



		interpret(terp, ast, NULL);

		printf("\n");
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

	free_cmdline(cmdline);
	free_interpreter(terp);
	return 0;
}

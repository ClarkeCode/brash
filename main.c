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



int main(/*int argc, char* argv[]*/) {
	char* program = "2 + 3 * 4";
	
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

	
	//Free allocated memory
	free(parser);
	free_astnode(ast);
	for (size_t x = 0; x < num_tok; x++) free((tok+x)->content);
	free(tok);
	free(lex);

	return 0;
}

#include <stdio.h>
#include <stdlib.h> //size_t
#include <stdbool.h> //bool
#include <string.h>

#include "structs.h"
#include "enumerations.h"
#include "enum_lookups.h"
#include "lexer.h"


//
typedef struct ASTNode_ {
	Token* token;
	struct ASTNode_* left;
	struct ASTNode_* right;
} ASTNode;

ASTNode* make_astnode(Token* token) {
	ASTNode* ast = (ASTNode*) calloc(1, sizeof(ASTNode));
	ast->token = token;
	ast->left = NULL;
	ast->right = NULL;
	return ast;
}

typedef struct {
	Token* feed_tape;
	size_t tape_size;
} Parser;

void consume(Parser* parser, token_t expected_token, char* unexpected_text) {
	Token* tok = (parser->feed_tape) + parser->tape_size;
	if (tok->type != expected_token) {
		//report error with unexpected
	}
	parser->tape_size++;
}
void parse_expression(Parser* parser) {}
void parse_statement(Parser* parser) {}
void parse_declaration(Parser* parser) {}


int main(int argc, char* argv[]) {
	char* program = " \t \"Hello this is a string\"\n +12.74 greg = true == false !!=<<=>>=   \n({}) -*/%&&||^^!";
	
	Lexer* lex = make_lexer(program, "notfile", 0);

	size_t num_tok = 0;
	Token* tok = lexer_tokenize_all(lex, &num_tok);
	
	for (size_t x = 0; x < num_tok; x++) {
		fprintf(stdout, "type: %-22s content: '%s'\n", getStr_token_t((tok+x)->type), (tok+x)->content);
	}
	fprintf(stdout, "Remaining in lexer: '%s'\n", lex->line + lex->char_offset);


	dump_lexer_tokens("lexer.out", tok, num_tok);

	
	//Free allocated memory
	for (size_t x = 0; x < num_tok; x++) free((tok+x)->content);
	free(tok);
	free(lex);

	return 0;
}

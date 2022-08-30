#include <stdio.h>
#include <stdlib.h> //size_t
#include <stdbool.h> //bool
#include <string.h>
#include <assert.h> //assert

#include "structs.h"
#include "enumerations.h"
#include "enum_lookups.h"
#include "lexer.h"

typedef enum {
	TYPE_NOT_VALUE,
	TYPE_BOOL,
	TYPE_NUMBER,
	TYPE_STRING
} Datatype;
bool is_valuetype(Datatype type) { return type == TYPE_BOOL || type == TYPE_NUMBER || type == TYPE_STRING; }


typedef struct ASTNode_ {
	Token* token;
	Datatype datatype;
	struct ASTNode_* parent;
	struct ASTNode_* left;
	struct ASTNode_* right;
} ASTNode;

size_t get_precedence(ASTNode* node) {
	assert(ALL_TOKENS == 24 && "Exhaustive handling of precedence");
	if (node->token->type == PAREN_OPEN)             { return 17; }
	//Placeholder: Bitwise Complement, Arithmetic Negation
	if (node->token->type == LOGICAL_NOT)            { return 16; }
	if (node->token->type == OPERATOR_MULTIPLY)      { return 15; }
	if (node->token->type == OPERATOR_DIVIDE)        { return 14; }
	if (node->token->type == OPERATOR_MODULO)        { return 13; }
	if (node->token->type == OPERATOR_ADD)           { return 12; }
	if (node->token->type == OPERATOR_SUB)           { return 11; }
	//Placeholder: Bitwise AND/OR/XOR
	if (node->token->type == LOGICAL_AND)            { return 10; }
	if (node->token->type == LOGICAL_OR)             { return 9; }
	if (node->token->type == LOGICAL_XOR)            { return 8; }
	if (node->token->type == OPERATOR_LESSER)        { return 7; }
	if (node->token->type == OPERATOR_GREATER)       { return 6; }
	if (node->token->type == OPERATOR_LESSER_EQUAL)  { return 5; }
	if (node->token->type == OPERATOR_GREATER_EQUAL) { return 4; }
	if (node->token->type == OPERATOR_EQUALITY)      { return 3; }
	if (node->token->type == OPERATOR_INEQUALITY)    { return 2; }
	if (node->token->type == OPERATOR_ASSIGNMENT)    { return 1; }
	return 0;
}

void dump_node(FILE* fp, ASTNode* ast);
void dump_tree(FILE* fp, ASTNode* rootNode);

ASTNode* make_astnode(Token* token) {
	if (!token) return NULL;
	ASTNode* ast = (ASTNode*) calloc(1, sizeof(ASTNode));
	ast->token = token;
	ast->parent = NULL;
	ast->left = NULL;
	ast->right = NULL;

	if (token->type == NUMBER) ast->datatype = TYPE_NUMBER;
	else if (token->type == BOOLEAN) ast->datatype = TYPE_BOOL;
	else if (token->type == STRING) ast->datatype = TYPE_STRING;
	else ast->datatype = TYPE_NOT_VALUE;
	return ast;
}
void free_astnode(ASTNode* node) {
	if (!node) return;
	free_astnode(node->left);
	free_astnode(node->right);
	if (node) free(node);
}
void hang_left(ASTNode* parent, ASTNode* child) {
	parent->left = child;
	child->parent = parent;
}
void hang_right(ASTNode* parent, ASTNode* child) {
	parent->right = child;
	child->parent = parent;
}
ASTNode* get_astroot(ASTNode* node) {
	if (!node) return NULL;
	ASTNode* root = node;
	while(true) {
		if (!node->parent) break;
		node = node->parent;
	}
	return root;
}

typedef struct {
	Token* feed_tape;
	size_t tape_size;
	size_t tape_offset;
} Parser;
Parser* make_parser(Token* tok, size_t num_tokens) {
	Parser* parser = (Parser*) calloc(1, sizeof(Parser));
	parser->feed_tape = tok;
	parser->tape_size = num_tokens;
	parser->tape_offset = 0;
	return parser;
}
Token* parser_current_token(Parser* parser) {
	if (parser->tape_offset >= parser->tape_size) return NULL;
	return (parser->feed_tape + parser->tape_offset);
}
void parser_advance_tape(Parser* parser) { parser->tape_offset++; }

void _insert_by_precedence(ASTNode* node, ASTNode* current_node) {
	ASTNode* greg = node;
}
ASTNode* parse_ast(Parser* parser, token_t terminator) {
	if (!parser || !parser->feed_tape || parser->tape_size == 0) return NULL;
	ASTNode* active_node = NULL;

	while(parser->tape_offset < parser->tape_size &&
			(parser_current_token(parser) && parser_current_token(parser)->type != terminator)) {
		Token* curr_token = parser_current_token(parser);
		ASTNode* curr_node = make_astnode(curr_token);
		
		if (!active_node) {
			active_node = curr_node;
		}
		//TODO: rotate by precedence
		else if (get_precedence(curr_node) > get_precedence(active_node)) {

		}
		else {
			hang_right(active_node, curr_node);
			active_node = curr_node;
		}

		parser_advance_tape(parser);
	}

	return get_astroot(active_node);
}


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

void dump_node(FILE* fp, ASTNode* ast) {
	if (!ast) fprintf(fp, "ERROR\n");
	fprintf(fp, "\t\"%s:%d:%d\" ", ast->token->origin_file, ast->token->origin_line, ast->token->origin_char);
	fprintf(fp, "[label=\"'%s'\\n%s\\n%s:%d:%d\"",
				ast->token->content, getStr_token_t(ast->token->type),
				ast->token->origin_file, ast->token->origin_line, ast->token->origin_char
			);
	fprintf(fp, "shape=\"%s\"]\n", true ? "ellipse" : "rectangle");

	if (ast->left) {
		fprintf(fp, "\t\"%s:%d:%d\"->\"%s:%d:%d\" [label=\"L\"]\n",
					ast->token->origin_file, ast->token->origin_line, ast->token->origin_char,
					ast->left->token->origin_file, ast->left->token->origin_line, ast->left->token->origin_char
				);
		dump_node(fp, ast->left);
	}
	if (ast->right) {
		fprintf(fp, "\t\"%s:%d:%d\"->\"%s:%d:%d\" [label=\"R\"]\n",
					ast->token->origin_file, ast->token->origin_line, ast->token->origin_char,
					ast->right->token->origin_file, ast->right->token->origin_line, ast->right->token->origin_char
				);
		dump_node(fp, ast->right);
	}
}
void dump_tree(FILE* fp, ASTNode* rootNode) {
	fprintf(fp, "digraph G {\n");
	dump_node(fp, rootNode);
	fprintf(fp, "}\n");
}

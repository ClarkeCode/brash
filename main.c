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
void hang_left(ASTNode* parent, ASTNode* child) {
	parent->left = child;
	child->parent = parent;
}
void hang_right(ASTNode* parent, ASTNode* child) {
	parent->right = child;
	child->parent = parent;
}
void overwrite_child(ASTNode* parent, ASTNode* oldchild, ASTNode* newchild) {
	if (parent->left == oldchild) hang_left(parent, newchild);
	else hang_right(parent, newchild);
}

typedef struct {
	Token* feed_tape;
	size_t tape_size;
	size_t tape_offset;
} Parser;

ASTNode* parse_ast(Parser* parser) {
	if (!parser || !parser->feed_tape || parser->tape_size == 0) return NULL;
	ASTNode* root_node = make_astnode(NULL);
	ASTNode* active_node = make_astnode(parser->feed_tape);
	hang_right(root_node, active_node);

	for (size_t x = 1; x < parser->tape_size; x++) {
		Token* tok = parser->feed_tape + x;
		ASTNode* new_node = make_astnode(tok);
		ASTNode* active_parent = active_node->parent;

		if (is_valuetype(active_node->datatype) &&  !is_valuetype(new_node->datatype)) {
			hang_left(new_node, active_node);
			overwrite_child(active_parent, active_node, new_node); //need different behaviour for unary
			active_node = new_node;
		}
		else if (!is_valuetype(active_node->datatype)) {
			if (active_node->left && active_node->right) {} //no room for operands, parse error
			else if (!active_node->left) {
				hang_left(active_node, new_node);
			}
			else if (!active_node->right) {
				hang_right(active_node, new_node);
				active_node = new_node;
			}
			else {} //Unreachable
		}
		else {
			//Unreachable?
		}
	}
	
	return root_node;
}


int main(/*int argc, char* argv[]*/) {
	char* program = "2 + 3 * 4";
	
	Lexer* lex = make_lexer(program, "notfile", 0);

	size_t num_tok = 0;
	Token* tok = lexer_tokenize_all(lex, &num_tok);
	
	for (size_t x = 0; x < num_tok; x++) {
		fprintf(stdout, "type: %-22s content: '%s'\n", getStr_token_t((tok+x)->type), (tok+x)->content);
	}
	fprintf(stdout, "Remaining in lexer: '%s'\n", lex->line + lex->char_offset);
	dump_lexer_tokens("lexer.out", tok, num_tok);

	ASTNode* test = make_astnode(tok);
	ASTNode* test1 = make_astnode(tok+1);
	ASTNode* test2 = make_astnode(tok+2);
	hang_right(test, test1);
	hang_left(test, test2);
	FILE* fp = fopen("ast.dump", "w");
	dump_tree(stdout, test);
	fclose(fp);


	get_precedence(test);

	
	//Free allocated memory
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

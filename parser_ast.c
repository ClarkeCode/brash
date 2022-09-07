#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h> //strtod, strcmp
#include "structs.h"
#include "parser_ast.h"
#include "enum_lookups.h"

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

void dump_node(FILE* fp, ASTNode* ast) {
	//printf("Node: %p\n\t%p %p %p\n\n", ast, ast->parent, ast->left, ast->right);
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


void _init_value_from_token(Value* val, Token* token) {
	switch (token->type) {
		case NUMBER:  {
				val->type = TYPE_NUMBER;
				val->as.number = strtod(token->content, NULL);
			} break;
		case BOOLEAN: {
				val->type = TYPE_BOOL;
				val->as.boolean = strcmp(token->content, "true") == 0 ? true : false;
			} break;
		case STRING:  {
				val->type = TYPE_STRING;
				val->as.string = token->content;
			} break;
		default:
				val->type = TYPE_NOT_VALUE;
	}
}

ASTNode* make_astnode(Token* token) {
	if (!token) return NULL;
	ASTNode* ast = (ASTNode*) calloc(1, sizeof(ASTNode));
	ast->token = token;
	ast->parent = NULL;
	ast->left = NULL;
	ast->right = NULL;

	//ast->value = 0;
	_init_value_from_token(&(ast->value), ast->token);
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
	ASTNode* root_candidate = node;
	while (root_candidate->parent) {
		root_candidate = root_candidate->parent;
	}
	return root_candidate;
}

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

ASTNode* _insert_by_precedence(ASTNode* active_node, ASTNode* current_node) {
	if (get_precedence(current_node) <= get_precedence(active_node)) {
		hang_right(active_node, current_node);
		return current_node;
	}

	if (!active_node->parent) {
		hang_left(current_node, active_node);
		return current_node;
	}
	return _insert_by_precedence(active_node->parent, current_node);
}
ASTNode* parse_ast(Parser* parser, token_t terminator) {
	if (!parser || !parser->feed_tape || parser->tape_size == 0) return NULL;
	ASTNode* active_node = NULL;

	while(parser->tape_offset < parser->tape_size &&
			(parser_current_token(parser) && parser_current_token(parser)->type != terminator)) {
		Token* curr_token = parser_current_token(parser);
		ASTNode* curr_node = make_astnode(curr_token);

		//First node
		if (!active_node) {
			active_node = curr_node;
		}
		else {
			active_node = _insert_by_precedence(active_node, curr_node);
		}

		parser_advance_tape(parser);
	}

	return get_astroot(active_node);
}

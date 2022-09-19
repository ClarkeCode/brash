#ifndef BRASH_PARSER_AST
#define BRASH_PARSER_AST
#include <stddef.h>
#include "structs.h"

size_t get_precedence(ASTNode* node);
ASTNode* make_astnode(Token* token);
void free_astnode(ASTNode* node);
void hang_left(ASTNode* parent, ASTNode* child);
void hang_right(ASTNode* parent, ASTNode* child);
ASTNode* get_astroot(ASTNode* node);

Parser* make_parser(Token* tok, size_t num_tokens);
Token* parser_current_token(Parser* parser);
void parser_advance_tape(Parser* parser);
ASTNode* _insert_by_precedence(ASTNode* active_node, ASTNode* current_node);
ASTNode* parse_ast(Parser* parser, token_t terminator);
#endif

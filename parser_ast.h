#ifndef BRASH_PARSER_AST
#define BRASH_PARSER_AST
#include <stddef.h>

typedef enum {
	TYPE_NOT_VALUE,
	TYPE_BOOL,
	TYPE_NUMBER,
	TYPE_STRING
} Datatype;

typedef struct {
	Token* feed_tape;
	size_t tape_size;
	size_t tape_offset;
} Parser;

typedef struct ASTNode_ {
	Token* token;
	Datatype datatype;
	struct ASTNode_* parent;
	struct ASTNode_* left;
	struct ASTNode_* right;
} ASTNode;



size_t get_precedence(ASTNode* node);
void dump_node(FILE* fp, ASTNode* ast);
void dump_tree(FILE* fp, ASTNode* rootNode);
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

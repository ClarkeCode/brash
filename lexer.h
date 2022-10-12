#ifndef BRASH_LEXER
#define BRASH_LEXER
typedef enum {
	TOKEN_ERROR,
	TOKEN_EOF,
	TOKEN_NEWLINE,

	STRING,
	NUMBER,
	BOOLEAN,
	IDENTIFIER,

	PAREN_OPEN,
	PAREN_CLOSE,
	BRACE_OPEN,
	BRACE_CLOSE,

	OPERATOR_ADD,
	OPERATOR_SUB,
	OPERATOR_MULTIPLY,
	OPERATOR_DIVIDE,
	OPERATOR_MODULO,

	OPERATOR_ASSIGNMENT,

	OPERATOR_EQUALITY,
	OPERATOR_INEQUALITY,
	OPERATOR_LESSER,
	OPERATOR_GREATER,
	OPERATOR_LESSER_EQUAL,
	OPERATOR_GREATER_EQUAL,

	LOGICAL_NOT,
	LOGICAL_AND,
	LOGICAL_OR,
	LOGICAL_XOR,

	ALL_TOKENS
} token_t;

typedef struct {
	char* original;
	size_t length;
} StrView;

char* unbox(StrView* strview);

typedef struct {
	char* file; //May be null when input is not from file; ie: REPL
	size_t line;
	size_t offset;
} Location;

void setLexer(char* filename, char* program);
token_t produceNextToken(StrView* content, Location* loc);
#endif

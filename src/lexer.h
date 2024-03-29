#ifndef BRASH_LEXER
#define BRASH_LEXER
typedef enum {
	TK_ERROR,
	TK_EOF,
	TK_NEWLINE,
	TK_SEMICOLON,
	TK_VAR,
	TK_IF,
	TK_ELSE,
	TK_PRINT,
	TK_WHILE,

	TK_STRING,
	TK_NUMBER,
	TK_BOOLEAN,
	TK_IDENTIFIER,

	TK_PAREN_OPEN,
	TK_PAREN_CLOSE,
	TK_BRACE_OPEN,
	TK_BRACE_CLOSE,

	TK_ADD,
	TK_SUB,
	TK_MULTIPLY,
	TK_DIVIDE,
	TK_MODULO,

	TK_ASSIGNMENT,

	TK_EQUALITY,
	TK_INEQUALITY,
	TK_LESSER,
	TK_GREATER,
	TK_LESSER_EQUAL,
	TK_GREATER_EQUAL,

	TK_NOT,
	TK_AND,
	TK_OR,
	TK_XOR,

	TK_DEC_FUNCTION,
	TK_RETURN,
	
	TK_PARAMETER_LIST_SEPARATOR,

	TK_TYPE_NUMBER,
	TK_TYPE_BOOLEAN,
	TK_TYPE_STRING,
	TK_TYPE_FUNCTION,
	TK_TYPE_CUSTOM,

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

typedef struct {
	token_t type;
	StrView content;
	Location location;
} Token;

void setLexer(char* filename, char* program);
Token produceNextToken();

bool isAssignmentStatement();
#endif

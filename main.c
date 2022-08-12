#include <stdio.h>
#include <stdlib.h> //size_t
#include <stdbool.h> //bool
#include <string.h>

//"Utility functions"
bool char_in(char search, char* searchspace) {
	for (size_t x = 0; searchspace[x] != '\0'; x++) {
		if (searchspace[x] == search) return true;
	}
	return false;
}

char* WHITESPACE = "\t\n ";
char* NUMERIC = "0123456789";
size_t advance_whitespace(size_t offset, char* source) {
	size_t advanced_offset = offset;
	for (; source[advanced_offset] != '\0'; advanced_offset++) {
		if (!char_in(source[advanced_offset], WHITESPACE)) return advanced_offset;
	}
	return advanced_offset;
}


//Enumerations and types
typedef enum {
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

	LOGICAL_NOT,
	LOGICAL_AND,
	LOGICAL_OR,
	LOGICAL_XOR

} token_t;

typedef struct {
	token_t type;
	char* content;

	char*  origin_file;
	size_t origin_line;
	size_t origin_char;

} Token;


typedef struct {
	char* line;
	char* source_file;
	size_t line_offset;
	size_t char_offset;
} Lexer;

Lexer* make_lexer(char* line, char* srcfile, size_t linenum) {
	Lexer* lex = (Lexer*) calloc(1, sizeof(Lexer));
	lex->line = line;
	lex->source_file = srcfile;
	lex->line_offset = linenum;
	lex->char_offset = 0;
	return lex;
}

void lexer_consume_fixed(Lexer* lexer, size_t chars_to_consume) {
	if (strlen(lexer->line + lexer->char_offset) > chars_to_consume)
		lexer->char_offset += chars_to_consume;
	else { //Cannot consume past the end of the line
		lexer->char_offset = strlen(lexer->line);
	}
}

Token* make_token(Lexer* lexer, token_t type, size_t contentlen) {
	Token* tok = (Token*) calloc(1, sizeof(Token));
	tok->type = type;
	tok->origin_file = lexer->source_file;
	tok->origin_line = lexer->line_offset;
	tok->origin_char = lexer->char_offset;

	tok->content = (char*) calloc(contentlen+1, sizeof(char));
	strncpy(tok->content, lexer->line + lexer->char_offset, contentlen);
	return tok;
}

//Primary tokenization function?
//Returning a nullptr means the lexer contains no more tokens
Token* lexer_produce_token(Lexer* lexer) {
	lexer->char_offset = advance_whitespace(lexer->char_offset, lexer->line);
	
	char lookbehind   = (lexer->char_offset > 0) ? lexer->line[lexer->char_offset-1] : '\0';
	char current_char = lexer->line[lexer->char_offset];
	char lookahead    = lexer->line[lexer->char_offset+1];

	Token* tok = NULL;
	
	if (current_char == '"') {
		lexer_consume_fixed(lexer, 1);
		//TODO: account for an escaped " character
		char* location_found = strchr(lexer->line + lexer->char_offset, '"');
		if (!location_found) {} //TODO: handle unmatched "
		size_t length = location_found - (lexer->line + lexer->char_offset);
		tok = make_token(lexer, STRING, length);
		lexer_consume_fixed(lexer, length + 1); //Advances past closing "
	}

	if (char_in(current_char, NUMERIC)) {
		char* orig = lexer->line + lexer->char_offset;
		char* found = orig;
		bool has_floating_point = false;
		while (*found != '\0') {
			if (*found >= '0' && *found <= '9') found++;
			else if (*found == '.' && !has_floating_point) {found++;has_floating_point=true;}
			else break;
		}
		size_t length = found - orig;
		tok = make_token(lexer, NUMBER, length);
		lexer_consume_fixed(lexer, length);
	}
	
	//When tokenizing keywords/operators, use strncmp
	else if (strncmp(lexer->line + lexer->char_offset, "+", strlen("+")) == 0) {
		tok = make_token(lexer, OPERATOR_ADD, strlen("+"));
		lexer_consume_fixed(lexer, strlen("+"));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "-", strlen("-")) == 0) {
		tok = make_token(lexer, OPERATOR_SUB, strlen("-"));
		lexer_consume_fixed(lexer, strlen("-"));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "*", strlen("*")) == 0) {
		tok = make_token(lexer, OPERATOR_MULTIPLY, strlen("*"));
		lexer_consume_fixed(lexer, strlen("*"));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "/", strlen("/")) == 0) {
		tok = make_token(lexer, OPERATOR_DIVIDE, strlen("/"));
		lexer_consume_fixed(lexer, strlen("/"));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "%", strlen("%")) == 0) {
		tok = make_token(lexer, OPERATOR_MODULO, strlen("%"));
		lexer_consume_fixed(lexer, strlen("%"));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "=", strlen("=")) == 0) {
		tok = make_token(lexer, OPERATOR_ASSIGNMENT, strlen("="));
		lexer_consume_fixed(lexer, strlen("="));
	}

	else if (strncmp(lexer->line + lexer->char_offset, "(", strlen("(")) == 0) {
		tok = make_token(lexer, PAREN_OPEN, strlen("("));
		lexer_consume_fixed(lexer, strlen("("));
	}
	else if (strncmp(lexer->line + lexer->char_offset, ")", strlen(")")) == 0) {
		tok = make_token(lexer, PAREN_CLOSE, strlen(")"));
		lexer_consume_fixed(lexer, strlen(")"));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "{", strlen("{")) == 0) {
		tok = make_token(lexer, BRACE_OPEN, strlen("{"));
		lexer_consume_fixed(lexer, strlen("{"));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "}", strlen("}")) == 0) {
		tok = make_token(lexer, BRACE_CLOSE, strlen("}"));
		lexer_consume_fixed(lexer, strlen("}"));
	}

	else if (strncmp(lexer->line + lexer->char_offset, "!", strlen("!")) == 0) {
		tok = make_token(lexer, LOGICAL_NOT, strlen("!"));
		lexer_consume_fixed(lexer, strlen("!"));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "&&", strlen("&&")) == 0) {
		tok = make_token(lexer, LOGICAL_AND, strlen("&&"));
		lexer_consume_fixed(lexer, strlen("&&"));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "||", strlen("||")) == 0) {
		tok = make_token(lexer, LOGICAL_OR, strlen("||"));
		lexer_consume_fixed(lexer, strlen("||"));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "^^", strlen("^^")) == 0) {
		tok = make_token(lexer, LOGICAL_XOR, strlen("^^"));
		lexer_consume_fixed(lexer, strlen("^^"));
	}

	else if (strncmp(lexer->line + lexer->char_offset, "true", strlen("true")) == 0) {
		tok = make_token(lexer, BOOLEAN, strlen("true"));
		lexer_consume_fixed(lexer, strlen("true"));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "false", strlen("false")) == 0) {
		tok = make_token(lexer, BOOLEAN, strlen("false"));
		lexer_consume_fixed(lexer, strlen("false"));
	}

	else if(char_in(current_char, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_")) {
		char* orig = lexer->line + lexer->char_offset; char* found = orig;
		while (*found != '\0') {
			if ((*found >= 'a' && *found <= 'z') ||
					(*found >= 'A' && *found <= 'Z') ||
					(*found == '-' || *found == '_') ||
					(*found >= '0' && *found <= '9')
				) found++;
			else break;
		}
		size_t length = found - orig;
		tok = make_token(lexer, IDENTIFIER, length);
		lexer_consume_fixed(lexer, length);
	}

	return tok;
}


char* getStr_token_t(token_t lookup) {
	switch(lookup) {
		case STRING              : return "STRING";
		case NUMBER              : return "NUMBER";
		case BOOLEAN             : return "BOOLEAN";
		case IDENTIFIER          : return "IDENTIFIER";
		case PAREN_OPEN          : return "PAREN_OPEN";
		case PAREN_CLOSE         : return "PAREN_CLOSE";
		case BRACE_OPEN          : return "BRACE_OPEN";
		case BRACE_CLOSE         : return "BRACE_CLOSE";
		case OPERATOR_ADD        : return "OPERATOR_ADD";
		case OPERATOR_SUB        : return "OPERATOR_SUB";
		case OPERATOR_MULTIPLY   : return "OPERATOR_MULTIPLY";
		case OPERATOR_DIVIDE     : return "OPERATOR_DIVIDE";
		case OPERATOR_MODULO     : return "OPERATOR_MODULO";
		case OPERATOR_ASSIGNMENT : return "OPERATOR_ASSIGNMENT";
		case LOGICAL_NOT         : return "LOGICAL_NOT";
		case LOGICAL_AND         : return "LOGICAL_AND";
		case LOGICAL_OR          : return "LOGICAL_OR";
		case LOGICAL_XOR         : return "LOGICAL_XOR";

		default: return "LOOKUP_UNKNOWN_VALUE";
	}
}


int main(int argc, char* argv[]) {
	char* program = " \t \"Hello this is a string\"\n +12.74 greg = true false   \n({}) -*/%&&||^^!";
	
	Lexer* lex = make_lexer(program, "notfile", 0);
	Token* tok = NULL;

	while ((tok = lexer_produce_token(lex)) ) {
		fprintf(stdout, "type: %-22s content: '%s'\n", getStr_token_t(tok->type), tok->content);
	}

	fprintf(stdout, "Remaining in lexer: '%s'\n", lex->line + lex->char_offset);

	return 0;
}

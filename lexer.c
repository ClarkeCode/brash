#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "structs.h"
#include "lexer.h"

Lexer* make_lexer(char* line, char* srcfile, size_t linenum) {
	Lexer* lex = (Lexer*) calloc(1, sizeof(Lexer));
	lex->line = line;
	lex->source_file = srcfile;
	lex->line_offset = linenum;
	lex->char_offset = 0;
	return lex;
}



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

	//Operators or keywords with potential overlaps
	else if (strncmp(lexer->line + lexer->char_offset, "==", strlen("==")) == 0) {
		tok = make_token(lexer, OPERATOR_EQUALITY, strlen("=="));
		lexer_consume_fixed(lexer, strlen("=="));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "!=", strlen("!=")) == 0) {
		tok = make_token(lexer, OPERATOR_INEQUALITY, strlen("!="));
		lexer_consume_fixed(lexer, strlen("!="));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "!", strlen("!")) == 0) {
		tok = make_token(lexer, LOGICAL_NOT, strlen("!"));
		lexer_consume_fixed(lexer, strlen("!"));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "=", strlen("=")) == 0) {
		tok = make_token(lexer, OPERATOR_ASSIGNMENT, strlen("="));
		lexer_consume_fixed(lexer, strlen("="));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "<=", strlen("<=")) == 0) {
		tok = make_token(lexer, OPERATOR_LESSER_EQUAL, strlen("<="));
		lexer_consume_fixed(lexer, strlen("<="));
	}
	else if (strncmp(lexer->line + lexer->char_offset, ">=", strlen(">=")) == 0) {
		tok = make_token(lexer, OPERATOR_GREATER_EQUAL, strlen(">="));
		lexer_consume_fixed(lexer, strlen(">="));
	}
	else if (strncmp(lexer->line + lexer->char_offset, "<", strlen("<")) == 0) {
		tok = make_token(lexer, OPERATOR_LESSER, strlen("<"));
		lexer_consume_fixed(lexer, strlen("<"));
	}
	else if (strncmp(lexer->line + lexer->char_offset, ">", strlen(">")) == 0) {
		tok = make_token(lexer, OPERATOR_GREATER, strlen(">"));
		lexer_consume_fixed(lexer, strlen(">"));
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


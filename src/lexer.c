#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "lexer.h"

//TODO: make lexer ignore comments


void nSetView(StrView* strview, char* orig, size_t len) {
	strview->original = orig;
	strview->length = len;
}
void setView(StrView* strview, char* orig) { nSetView(strview, orig, strlen(orig)); }

char* unbox(StrView* strview) {
	char* str = (char*) calloc(strview->length + 1, sizeof(char));
	strncpy(str, strview->original, strview->length);
	return str;
}

typedef struct {
	char* start;
	char* current;
	Location location;
} Lexer;
Lexer lexer;

void setLexer(char* filename, char* program) {
	lexer.start = program;
	lexer.current = program;
	lexer.location.file = filename;
	lexer.location.line = 0;
	lexer.location.offset = 0;
}

char currentChar() { return *lexer.current; }
bool atEnd() { return currentChar() == '\0'; }
bool isNewline() { return currentChar() == '\n'; }

bool char_in(char search, char* searchspace) {
	for (size_t x = 0; searchspace[x] != '\0'; x++) {
		if (searchspace[x] == search) return true;
	}
	return false;
}
void advance_whitespace(char** sourcePtr) {
	while ((*sourcePtr)[0] != '\0' && char_in((*sourcePtr)[0], " \t")) {
		(*sourcePtr)++;
		lexer.location.offset++;
	}
}

//♪♫ Won't - you - take - me - to ♪♫ MACRO TOWN!
#define SET_CONTENT(msg) setView(content, msg)
#define LEXER_ERROR_MSG(msg) setView(content, msg)
#define MATCHES(str, desired) strncmp(str, desired, strlen(desired)) == 0
#define ADVANCE_LEXER(amount) lexer.current += amount; lexer.location.offset += amount
#define RET_IF_MATCH(desired, tok_t)\
	if (MATCHES(lexer.current, desired)) { SET_CONTENT(desired); ADVANCE_LEXER(strlen(desired)); return tok_t; }

char* VALID_NUMERIC_CHAR = "0123456789.";
char* VALID_IDENTIFIER_CHAR = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_";
token_t _produceNextToken(StrView* content, Location* loc) {
	advance_whitespace(&lexer.current);
	lexer.start = lexer.current;

	while (currentChar() == '#' || (*lexer.current == '/' && *(lexer.current+1) == '/')) {
		while (currentChar() != '\n') {
			ADVANCE_LEXER(1);
		}
		ADVANCE_LEXER(1);
		lexer.location.line++;
		lexer.location.offset = 0;
	}
	*loc = lexer.location; //Copy by-value


	if (atEnd()) {
		SET_CONTENT("");
		return TK_EOF;
	}

	//Variable length
	if (isNewline()) {
		lexer.location.line++;
		lexer.location.offset = 0;
		lexer.current += 1;
		SET_CONTENT("");
		return TK_NEWLINE;
	}
	if (currentChar() == '"') { //TODO: handle escaped " characters
		lexer.start++;
		char* location_found = strchr(lexer.start, '"');
		if (!location_found) {
			LEXER_ERROR_MSG("Missing a closing '\"' to properly terminate string.");
			return TK_ERROR;
		}
		lexer.current = location_found+1;
		nSetView(content, lexer.start, lexer.current-lexer.start-1);
		lexer.location.offset += (lexer.current-lexer.start)+1;
		return TK_STRING;
	}
	if (char_in(currentChar(), VALID_NUMERIC_CHAR)) {
		bool has_floating_point = false;
		while (currentChar() != '\0' && char_in(currentChar(), VALID_NUMERIC_CHAR)) {
			if (has_floating_point && currentChar() == '.') {
				LEXER_ERROR_MSG("Valid numbers cannot have multiple decimal places.");
				lexer.current++;
				return TK_ERROR;
			}
			if (!has_floating_point && currentChar() == '.') { has_floating_point = true; }
			lexer.current++;
		}
		nSetView(content, lexer.start, lexer.current-lexer.start);
		lexer.location.offset += (lexer.current-lexer.start);
		return TK_NUMBER;
	}

	//Fixed length
	RET_IF_MATCH(";",       TK_SEMICOLON);
	RET_IF_MATCH("var",     TK_VAR);
	RET_IF_MATCH("if",      TK_IF);
	RET_IF_MATCH("else",    TK_ELSE);
	RET_IF_MATCH("print",   TK_PRINT);
	RET_IF_MATCH("while",   TK_WHILE);
	RET_IF_MATCH("func",    TK_DEC_FUNCTION);
	RET_IF_MATCH(":",       TK_PARAMETER_LIST_SEPARATOR);
	RET_IF_MATCH("Number",  TK_TYPE_NUMBER);
	RET_IF_MATCH("Boolean", TK_TYPE_BOOLEAN);
	RET_IF_MATCH("Bool",    TK_TYPE_BOOLEAN);
	RET_IF_MATCH("String",  TK_TYPE_STRING);
	RET_IF_MATCH("type",    TK_TYPE_CUSTOM);
	RET_IF_MATCH("return",  TK_RETURN);



	RET_IF_MATCH("+", TK_ADD);
	RET_IF_MATCH("-", TK_SUB);
	RET_IF_MATCH("*", TK_MULTIPLY);
	RET_IF_MATCH("/", TK_DIVIDE);
	RET_IF_MATCH("%", TK_MODULO);

	RET_IF_MATCH("(", TK_PAREN_OPEN);
	RET_IF_MATCH(")", TK_PAREN_CLOSE);
	RET_IF_MATCH("{", TK_BRACE_OPEN);
	RET_IF_MATCH("}", TK_BRACE_CLOSE);

	RET_IF_MATCH("&&",    TK_AND);
	RET_IF_MATCH("||",    TK_OR);
	RET_IF_MATCH("^^",    TK_XOR);
	RET_IF_MATCH("true",  TK_BOOLEAN);
	RET_IF_MATCH("false", TK_BOOLEAN);

	//Operators or keywords with potential overlaps
	RET_IF_MATCH("==", TK_EQUALITY);
	RET_IF_MATCH("!=", TK_INEQUALITY);
	RET_IF_MATCH("!",  TK_NOT);
	RET_IF_MATCH("=",  TK_ASSIGNMENT);
	RET_IF_MATCH("<=", TK_LESSER_EQUAL);
	RET_IF_MATCH(">=", TK_GREATER_EQUAL);
	RET_IF_MATCH("<",  TK_LESSER);
	RET_IF_MATCH(">",  TK_GREATER);

	if (char_in(currentChar(), VALID_IDENTIFIER_CHAR)) {
		while (currentChar() != '\0' && char_in(currentChar(), VALID_IDENTIFIER_CHAR)) {
			lexer.current++;
		}
		lexer.current++;
		nSetView(content, lexer.start, lexer.current-lexer.start-1);
		lexer.location.offset += (lexer.current-lexer.start);
		return TK_IDENTIFIER;
	}

	LEXER_ERROR_MSG("Unexpected character.");
	return TK_ERROR;
}
#undef SET_CONTENT
#undef LEXER_ERROR_MSG
#undef MATCHES
#undef ADVANCE_LEXER
#undef RET_IF_MATCH


#include "debugging.h"
#include "enum_lookups.h"

Token produceNextToken() {
	Token nextToken = {0};
	nextToken.type = _produceNextToken(&nextToken.content, &nextToken.location);
	printDebug(stdout, LEX_TOKEN_PRODUCTION, "[LEX] produceNextToken: %s\n", getStr_token_t(nextToken.type));
	return nextToken;
}



typedef struct {
	char* start;
	char* current;
	Location location;
} LexerState;

LexerState getLexerState() {
	LexerState state = {0};
	state.start = lexer.start;
	state.current = lexer.current;
	state.location = lexer.location;
	return state;
}
void setLexerState(LexerState state) {
	lexer.start = state.start;
	lexer.current = state.current;
	lexer.location = state.location;
}


bool isAssignmentStatement() {
	LexerState state = getLexerState();
	bool hasAssignment = false;

	//Keep producing tokens until you find an assignment (true) or a terminator (false);
	while (true) {
		Token tk = produceNextToken();
		if (tk.type == TK_ASSIGNMENT) {
			hasAssignment = true;
			break;
		}
		if (tk.type == TK_EOF ||
				tk.type == TK_NEWLINE ||
				tk.type == TK_SEMICOLON ||
				tk.type == TK_BRACE_OPEN ||
				tk.type == TK_BRACE_CLOSE) {
			hasAssignment = false;
			break;
		}
	}
	setLexerState(state);
	return hasAssignment;
}

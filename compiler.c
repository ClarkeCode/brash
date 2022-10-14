#include <stdio.h>
#include <stdlib.h> //size_t
#include <string.h>

#include "compiler.h"
#include "enum_lookups.h"
#include "lexer.h" //Token

typedef struct {
	Token current;
	Token previous;

	bool hadError;
	bool panicMode;
} Parser;

Parser parser;
Chunk* compilingChunk;
static Chunk* currentChunk() { return compilingChunk; }

//Parser error functions
bool _tokenHasContent(Token* token) { return token->content.length != 0; }
void errorAt(Token* token, const char* message) {
	if (parser.panicMode) return;
	parser.panicMode = true;

	FILE* fp = stderr;
	if (token->location.file)
		fprintf(fp, "[%s:%d:%d] ", token->location.file, token->location.line, token->location.offset);
	else
		fprintf(fp, "[%d:%d] ", token->location.line, token->location.offset);
	fprintf(fp, "Syntax Error: ");

	if (message)
		fprintf(fp, "%s", message);
	if (!_tokenHasContent(token)) {
		char* tokencontent = unbox(&(token->content));
		fprintf(fp, "%s'%s'", ((message) ? ": " : ""), tokencontent);
		free(tokencontent);
	}
	fprintf(fp, "\n");
	parser.hadError = true;
}
void errorAtCurrent(const char* message) { errorAt(&parser.current, message); }
void errorAtPrevious(const char* message) { errorAt(&parser.previous, message); }


void advanceParser() {
	parser.previous = parser.current;
	parser.current = produceNextToken();
	if (parser.current.type == TK_ERROR)
		errorAtCurrent(NULL);
}
void consumeIf(token_t type, const char* failureMessage) {
	if (parser.current.type == type) {
		advanceParser(); return;
	}
	errorAtCurrent(failureMessage);
}



static void emitByte(byte_t byte) {
  writeChunk(currentChunk(), byte, parser.previous.location);
}
void emitBytes(byte_t b1, byte_t b2) {
	emitByte(b1);
	emitByte(b2);
}
void emitReturn() { emitByte(OP_RETURN); }
void emitConstant(Value value) {
	//consumed makeConstant
	size_t index = recordConstant(currentChunk(), value);
	if (index > UINT8_MAX) {
		fprintf(stderr, "Too many constants in a single chunk");
		return;
	}
	emitBytes(OP_CONSTANT, (byte_t)index);
}
void endCompiler() { emitReturn(); }




typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,
  PREC_OR,
  PREC_AND,
  PREC_XOR,
  PREC_EQUALITY,
  PREC_COMPARISON,
  PREC_TERM,
  PREC_FACTOR,
  PREC_UNARY,
  PREC_CALL,
  PREC_PRIMARY
} Precedence;
typedef void (*ParseFn)(bool canAssign);
typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;
/*
ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, call,   PREC_CALL},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     dot,    PREC_CALL},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {unary,    NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_LESS]          = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_IDENTIFIER]    = {variable, NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     and_,   PREC_AND},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {literal,  NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     or_,    PREC_OR},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {super_,   NULL,   PREC_NONE},
  [TOKEN_THIS]          = {this_,    NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};
*/

//forward declarations
void binary(bool canAssign);
void parsePrecedence(Precedence precedence);

void number(bool canAssign) {
	Value value = {0};
	value.type = VAL_NUMBER;
	char* numstring = unbox(&parser.previous.content);
	value.as.number = strtod(numstring, NULL);
	free(numstring);
	emitConstant(value);
}
void expression(bool canAssign) {
	parsePrecedence(PREC_ASSIGNMENT);
}
void grouping(bool canAssign) {
	expression(canAssign);
	consumeIf(TK_PAREN_CLOSE, "Expected a closing ')' after expression");
}
void unary(bool canAssign) {
	token_t operatorType = parser.previous.type;
	parsePrecedence(PREC_UNARY);

	switch (operatorType) {
		case TK_SUB:
			emitByte(OP_NEGATE);
			break;
		case TK_NOT:
			emitByte(OP_NOT);
			break;
		default: return; //Unreachable
	}
}


ParseRule rules[] = {
	[TK_ERROR]         = {NULL,     NULL,   PREC_NONE},
	[TK_EOF]           = {NULL,     NULL,   PREC_NONE},
	[TK_NEWLINE]       = {NULL,     NULL,   PREC_NONE},
	[TK_STRING]        = {NULL,     NULL,   PREC_NONE},
	[TK_NUMBER]        = {number,   NULL,   PREC_NONE},
	[TK_BOOLEAN]       = {NULL,     NULL,   PREC_NONE},
	[TK_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
	[TK_PAREN_OPEN]    = {grouping, NULL,   PREC_NONE},
	[TK_PAREN_CLOSE]   = {NULL,     NULL,   PREC_NONE},
	[TK_BRACE_OPEN]    = {NULL,     NULL,   PREC_NONE},
	[TK_BRACE_CLOSE]   = {NULL,     NULL,   PREC_NONE},
	[TK_ADD]           = {NULL,     binary, PREC_TERM},
	[TK_SUB]           = {unary,    binary, PREC_TERM},
	[TK_MULTIPLY]      = {NULL,     binary, PREC_FACTOR},
	[TK_DIVIDE]        = {NULL,     binary, PREC_FACTOR},
	[TK_MODULO]        = {NULL,     binary, PREC_FACTOR},
	[TK_ASSIGNMENT]    = {NULL,     NULL,   PREC_NONE},
	[TK_EQUALITY]      = {NULL,     binary, PREC_EQUALITY},
	[TK_INEQUALITY]    = {NULL,     binary, PREC_EQUALITY},
	[TK_LESSER]        = {NULL,     binary, PREC_COMPARISON},
	[TK_GREATER]       = {NULL,     binary, PREC_COMPARISON},
	[TK_LESSER_EQUAL]  = {NULL,     binary, PREC_COMPARISON},
	[TK_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
	[TK_NOT]           = {unary,    NULL,   PREC_NONE},
	[TK_AND]           = {NULL,     NULL,   PREC_AND},
	[TK_OR]            = {NULL,     NULL,   PREC_OR},
	[TK_XOR]           = {NULL,     NULL,   PREC_XOR},
};
ParseRule* getRule(token_t type) { return &rules[type]; }

void binary(bool canAssign) {
	token_t operatorType = parser.previous.type;
	switch (operatorType) {
		case TK_ADD:           emitByte(OP_ADD); break;
		case TK_SUB:           emitByte(OP_SUBTRACT); break;
		case TK_MULTIPLY:      emitByte(OP_MULTIPLY); break;
		case TK_DIVIDE:        emitByte(OP_DIVIDE); break;
		case TK_MODULO:        emitByte(OP_MODULO); break;

		case TK_EQUALITY:      emitByte(OP_EQUALS); break;
		case TK_INEQUALITY:    emitBytes(OP_EQUALS, OP_NOT); break;
		case TK_LESSER:        emitByte(OP_LESSER); break;
		case TK_GREATER:       emitByte(OP_GREATER); break;
		case TK_LESSER_EQUAL:  emitBytes(OP_GREATER, OP_NOT); break;
		case TK_GREATER_EQUAL: emitBytes(OP_LESSER, OP_NOT); break;
		default: return; //Unreachable
	}
}
void parsePrecedence(Precedence precedence) {
	advanceParser();
	ParseFn prefixRule = getRule(parser.previous.type)->prefix;
	if (prefixRule == NULL) {
		fprintf(stderr, "Expected an expression");
		return;
	}

	bool canAssign = precedence <= PREC_ASSIGNMENT;
	prefixRule(canAssign);

	while (precedence <= getRule(parser.current.type)->precedence) {
		advanceParser();
		ParseFn infixRule = getRule(parser.previous.type)->infix;
		infixRule(canAssign);
	}
}

bool compile(const char* source, Chunk* chunk) {
	setLexer("notafile", (char*)source);
	compilingChunk = chunk;
	parser.hadError = false;
	parser.panicMode = false;
	advanceParser();
	expression(false);
	consumeIf(TK_EOF, "Expected end of expression.");
	endCompiler();
	return true;
}

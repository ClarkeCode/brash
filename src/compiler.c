#include <stdio.h>
#include <stdlib.h> //size_t
#include <string.h>

#include "compiler.h"
#include "enum_lookups.h"

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



//Function registry
typedef struct {
	char** strings;
	size_t size;
	size_t capacity;
} RegisteredFunctions; //Unordered string set
RegisteredFunctions rgFunc;

bool isFuncRegistered(char* name) {
	for (size_t x = 0; x < rgFunc.size; x++) {
		if (strcmp(name, rgFunc.strings[x]) == 0)
			return true;
	}
	return false;
}
void registerFunc(char* name) {
	if (rgFunc.size >= rgFunc.capacity) {
		rgFunc.capacity *= 2;
		rgFunc.strings = realloc(rgFunc.strings, rgFunc.capacity * sizeof(char*));
	}
	rgFunc.strings[rgFunc.size] = malloc(strlen(name) + 1);
	strcpy(rgFunc.strings[rgFunc.size], name);
	rgFunc.size++;
}
void makeRegistry() {
	rgFunc.size = 0;
	rgFunc.capacity = 16;
	rgFunc.strings = malloc(sizeof(char*) * rgFunc.capacity);
}
void freeRegistry() {
	for (size_t x = 0; x < rgFunc.size; x++) {
		if (rgFunc.strings[x]) free(rgFunc.strings[x]);
	}
	if (rgFunc.strings) free(rgFunc.strings);
}





bool checkIf(token_t type) { return parser.current.type == type; }
void advanceParser() {
	parser.previous = parser.current;
	parser.current = produceNextToken();
	if (parser.current.type == TK_ERROR)
		errorAtCurrent(NULL);
}
void consumeIf(token_t type, const char* failureMessage) {
	if (checkIf(type)) {
		advanceParser(); return;
	}
	errorAtCurrent(failureMessage);
}
void consumeIfMultiple(token_t* typeArr, size_t arrSize, const char* failureMessage) {
	for (size_t x = 0; x < arrSize; x++) {
		if (checkIf(typeArr[x])) {
			advanceParser(); return;
		}
	}
	errorAtCurrent(failureMessage);
}
bool match(token_t type) {
	if (!checkIf(type)) return false;
	advanceParser();
	return true;
}


static void emitByte(byte_t byte) {
	writeChunk(currentChunk(), byte, parser.previous.location);
}
void emitBytes(byte_t b1, byte_t b2) {
	emitByte(b1);
	emitByte(b2);
}
void emitReturn() { emitByte(OP_RETURN); }
void emitDouble(double toBundle) {
	double bundledNumber = 0;
	transpose8Bytes(&toBundle, &bundledNumber);
	for (size_t x = 0; x < 8; x++) {
		emitByte(((byte_t*)&bundledNumber)[x]);
	}
}
void emitString(char* string) {
	for (size_t x = 0; x < strlen(string)+1; x++) { //Also emits the terminating null byte
		emitByte((byte_t) string[x]);
	}
}
void endCompiler() {
	emitReturn();
	freeRegistry();
}
size_t emitJump(byte_t byte) {
	size_t offset = currentChunk()->size;
	emitByte(byte);
	emitBytes(0xff, 0xff);
	return offset;
}
void patchJump(size_t jumpOffset) {
	size_t currentOffset = currentChunk()->size;
	if ((currentOffset-jumpOffset) > INT16_MAX) {
		fprintf(stderr, "Cannot jump; too many instructions.");
		exit(EXIT_FAILURE);
	}
	int16_t bundledNumber = currentOffset - jumpOffset;
	transpose2Bytes(&bundledNumber, currentChunk()->code + jumpOffset + 1);
}

void emitLoop(size_t loopStart) {
	size_t currentOffset = currentChunk()->size;
	emitByte(OP_LOOP);
	if ((currentOffset-loopStart) > INT16_MAX) {
		fprintf(stderr, "Cannot jump; too many instructions.");
		exit(EXIT_FAILURE);
	}

	int16_t toBundle = currentOffset - loopStart;
	emitBytes(0xff, 0xff); //Dummy bytes
	transpose2Bytes(&toBundle, currentChunk()->code + currentChunk()->size - 2); //Patch the values with the right values in proper endian-ness
}




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
void expression();
void declaration();
void binary(bool canAssign);
void parsePrecedence(Precedence precedence);
void expression();
void statement();
void block();
void variableAssignment();

void number(bool canAssign) {
	char* numstring = unbox(&parser.previous.content);
	double num = strtod(numstring, NULL);
	free(numstring);
	emitByte(OP_NUMBER);
	emitDouble(num);
}
void identifier(bool canAssign) {
	char* stringval = unbox(&parser.previous.content);
	emitByte(OP_GET_VARIABLE);
	emitString(stringval);
	free(stringval);
}
void string(bool canAssign) {
	char* stringval = unbox(&parser.previous.content);
	emitByte(OP_STRING);
	emitString(stringval);
	free(stringval);
}
void boolean(bool canAssign) {
	char* boolstring = unbox(&parser.previous.content);
	emitByte((strcmp(boolstring, "true")==0) ? OP_TRUE : OP_FALSE);
	free(boolstring);
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
	[TK_ERROR]         = {NULL,        NULL,   PREC_NONE},
	[TK_EOF]           = {NULL,        NULL,   PREC_NONE},
	[TK_NEWLINE]       = {NULL,        NULL,   PREC_NONE},
	[TK_IF]            = {NULL,        NULL,   PREC_NONE},
	[TK_ELSE]          = {NULL,        NULL,   PREC_NONE},
	[TK_STRING]        = {string,      NULL,   PREC_NONE},
	[TK_NUMBER]        = {number,      NULL,   PREC_NONE},
	[TK_BOOLEAN]       = {boolean,     NULL,   PREC_NONE},
	[TK_IDENTIFIER]    = {identifier,  NULL,   PREC_NONE},
	[TK_PAREN_OPEN]    = {grouping,    NULL,   PREC_NONE},
	[TK_PAREN_CLOSE]   = {NULL,        NULL,   PREC_NONE},
	[TK_BRACE_OPEN]    = {NULL,        NULL,   PREC_NONE},
	[TK_BRACE_CLOSE]   = {NULL,        NULL,   PREC_NONE},
	[TK_ADD]           = {NULL,        binary, PREC_TERM},
	[TK_SUB]           = {unary,       binary, PREC_TERM},
	[TK_MULTIPLY]      = {NULL,        binary, PREC_FACTOR},
	[TK_DIVIDE]        = {NULL,        binary, PREC_FACTOR},
	[TK_MODULO]        = {NULL,        binary, PREC_FACTOR},
	[TK_ASSIGNMENT]    = {NULL,        NULL,   PREC_NONE},
	[TK_EQUALITY]      = {NULL,        binary, PREC_EQUALITY},
	[TK_INEQUALITY]    = {NULL,        binary, PREC_EQUALITY},
	[TK_LESSER]        = {NULL,        binary, PREC_COMPARISON},
	[TK_GREATER]       = {NULL,        binary, PREC_COMPARISON},
	[TK_LESSER_EQUAL]  = {NULL,        binary, PREC_COMPARISON},
	[TK_GREATER_EQUAL] = {NULL,        binary, PREC_COMPARISON},
	[TK_NOT]           = {unary,       NULL,   PREC_NONE},
	[TK_AND]           = {NULL,        binary, PREC_AND},
	[TK_OR]            = {NULL,        binary, PREC_OR},
	[TK_XOR]           = {NULL,        binary, PREC_XOR},
};
ParseRule* getRule(token_t type) { return &rules[type]; }

void binary(bool canAssign) {
	token_t operatorType = parser.previous.type;
	ParseRule* rule = getRule(operatorType);
	parsePrecedence((Precedence)(rule->precedence + 1));

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

		case TK_NOT:           emitByte(OP_NOT); break;
		case TK_AND:           emitByte(OP_AND); break;
		case TK_OR:            emitByte(OP_OR); break;
		case TK_XOR:           emitByte(OP_XOR); break;
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

void functionDefinition() {
	emitByte(OP_DEC_FUNCTION);

	advanceParser();
	char* funcname = unbox(&parser.previous.content);
	emitString(funcname);
	if (!isFuncRegistered(funcname)) {
		registerFunc(funcname);
	}
	else {
		//Function redefinition?
	}
	free(funcname);

	//Read parameters
	Types paramTypes[255] = {0};
	char* paramNames[255];
	size_t arity = 0;
	while (!checkIf(TK_BRACE_OPEN) && !checkIf(TK_PARAMETER_LIST_SEPARATOR)) {
		if (parser.current.type >= TK_TYPE_NUMBER && parser.current.type <= TK_TYPE_CUSTOM) {
			switch (parser.current.type) {
				case TK_TYPE_NUMBER:
					paramTypes[arity] = TYPE_NUMBER;
					break;
				case TK_TYPE_BOOLEAN:
					paramTypes[arity] = TYPE_BOOLEAN;
					break;
				case TK_TYPE_STRING:
					paramTypes[arity] = TYPE_STRING;
					break;
				case TK_TYPE_FUNCTION:
				case TK_TYPE_CUSTOM:
					break;
				default:
					break;
			}
		}
		else {
			//Problem!!
		}

		advanceParser();
		if (checkIf(TK_IDENTIFIER)) {
			paramNames[arity] = unbox(&parser.current.content);
		}
		else {
			//Problem!!
		}
		arity++;
		advanceParser();
	}

	//Read return types
	Types returnTypes[255] = {0};
	size_t rarity = 0;
	if (checkIf(TK_PARAMETER_LIST_SEPARATOR)) {
		advanceParser();
		while (!checkIf(TK_BRACE_OPEN)) {
			if (parser.current.type >= TK_TYPE_NUMBER && parser.current.type <= TK_TYPE_CUSTOM) {
				switch (parser.current.type) {
					case TK_TYPE_NUMBER:
						returnTypes[rarity] = TYPE_NUMBER;
						break;
					case TK_TYPE_BOOLEAN:
						returnTypes[rarity] = TYPE_BOOLEAN;
						break;
					case TK_TYPE_STRING:
						returnTypes[rarity] = TYPE_STRING;
						break;
					case TK_TYPE_FUNCTION:
					case TK_TYPE_CUSTOM:
						break;
					default:
						break;
				}
			}
			else {
				//Problem!!
			}
			rarity++;
			advanceParser();
		}
	}


	//Emit information
	emitByte((byte_t) arity);
	for (size_t x = 0; x < arity; x++) {
		emitByte((byte_t) paramTypes[x]);
	}
	for (size_t x = 0; x < arity; x++) {
		emitString(paramNames[x]);
		if (paramNames[x]) free(paramNames[x]);
	}

	emitByte((byte_t) rarity);
	for (size_t x = 0; x < rarity; x++) {
		emitByte((byte_t) returnTypes[x]);
	}

	consumeIf(TK_BRACE_OPEN, "Expected a block opening");
	block();
}

void expression() { parsePrecedence(PREC_ASSIGNMENT); }
void expressionStatement() {
	expression();
	token_t terminators[] = {TK_EOF, TK_NEWLINE, TK_SEMICOLON, TK_BRACE_OPEN};
	consumeIfMultiple(terminators, 4, "Expected an expression terminator");
	emitByte(OP_POP);
}
void block() {
	while (match(TK_NEWLINE)) {} //Consume any newlines
	emitByte(OP_ENTER_SCOPE);
	while (!checkIf(TK_BRACE_CLOSE) && !checkIf(TK_EOF)) {
		declaration();
	}
	consumeIf(TK_BRACE_CLOSE, "Expect '}' after block.");
	emitByte(OP_EXIT_SCOPE);
	while (match(TK_NEWLINE)) {} //Consume any newlines
}
void statement() {
	if (match(TK_IF)) { //If statement
		expression();

		size_t jumpOffset = emitJump(OP_JUMP_IF_FALSE);
		statement();

		if (match(TK_ELSE)) {
			size_t elseOffset = emitJump(OP_JUMP);
			patchJump(jumpOffset);
			statement();
			patchJump(elseOffset);
		}
		else {
			patchJump(jumpOffset);
		}
	}

	else if (match(TK_WHILE)) { //While statement
		size_t loopStart = currentChunk()->size;
		expression();

		size_t exitJump = emitJump(OP_JUMP_IF_FALSE);
		statement();

		emitLoop(loopStart);
		patchJump(exitJump);
	}

	else if (match(TK_PRINT)) { //Print statement
		expression();
		emitByte(OP_PRINT);
	}

	else if (match(TK_DEC_FUNCTION)) {
		functionDefinition();
	}

	else if (match(TK_BRACE_OPEN))
		block();
	else {
		expressionStatement();
	}
	while (match(TK_NEWLINE)) {} //Consume any newlines
}

void variableAssignment() {
	char* identString = unbox(&parser.previous.content);
	consumeIf(TK_ASSIGNMENT, "Requires assignment operator.");
	expression();

	token_t terminators[] = {TK_EOF, TK_NEWLINE, TK_SEMICOLON, TK_BRACE_OPEN};
	consumeIfMultiple(terminators, 4, "Expected an expression terminator.");
	emitByte(OP_SET_VARIABLE);
	emitString(identString);

	free(identString);
}

void variableDeclaration() {
	consumeIf(TK_IDENTIFIER, "Expected variable name.");
	char* identString = unbox(&parser.previous.content);
	emitByte(OP_DEF_VARIABLE);
	emitString(identString);

	if (checkIf(TK_ASSIGNMENT)) {
		variableAssignment();
	}

	free(identString);
}
void declaration() {
	while (match(TK_NEWLINE)) {} //Consume any newlines
	if (match(TK_EOF)) return;
	else if (match(TK_VAR)) {
		variableDeclaration();
	}
	else if (match(TK_IDENTIFIER)) {
		char* idenName = unbox(&parser.previous.content);
		if (isFuncRegistered(idenName)) { //TODO: if the identifier is a defined function, then this is a function call
			emitByte(OP_FUNCTION_CALL);
			emitString(idenName);
		}
		else {
			variableAssignment();
		}
		free(idenName);
	}
	else
		statement();
//	if (parser.panicMode) synchronize();
}




bool compile(const char* source, Chunk* chunk) {
	setLexer("notafile", (char*)source);
	compilingChunk = chunk;
	makeRegistry();
	parser.hadError = false;
	parser.panicMode = false;
	advanceParser();

	while (!match(TK_EOF)) {
		declaration();
	}
	consumeIf(TK_EOF, "Expected end of expression.");
	endCompiler();
	return true;
}

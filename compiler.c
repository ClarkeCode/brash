#include <stdio.h>
#include <stdlib.h> //size_t
#include <string.h>

#include "compiler.h"

#include "enum_lookups.h"

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
		char* tokencontent = unbox(&token->content);
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
	parser.current.type = produceNextToken(&parser.current.content, &parser.current.location);
	if (parser.current.type == TOKEN_ERROR)
		errorAtCurrent(NULL);
}
void consumeIf(token_t type, const char* failureMessage) {
	if (parser.current.type == type) {
		advanceParser(); return;
	}
	errorAtCurrent(failureMessage);
}



static void emitByte(byte_t byte) {
  writeChunk(currentChunk(), byte/*, parser.previous.line*/);
}
void emitReturn() { emitByte(OP_RETURN); }
void endCompiler() { emitReturn(); }


bool compile(const char* source, Chunk* chunk) {
	setLexer("notafile", (char*)source);
	compilingChunk = chunk;
	parser.hadError = false;
	parser.panicMode = false;
	advanceParser();
	//expression
	consumeIf(TOKEN_EOF, "Expected end of expression.");
	endCompiler();
	return true;
}

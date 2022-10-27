#ifndef BRASH_CHUNK
#define BRASH_CHUNK
#include <stddef.h>
#include <stdint.h>
#include "value.h"
#include "lexer.h" //Location
#include "bytes.h"

typedef enum {
	OP_RETURN,
	OP_NUMBER,
	OP_STRING,
	OP_NEGATE,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_MODULO,

	OP_TRUE,
	OP_FALSE,
	OP_EQUALS,
	OP_GREATER,
	OP_LESSER,

	OP_NOT,
	OP_AND,
	OP_OR,
	OP_XOR,

	OP_DEF_VARIABLE,
	OP_SET_VARIABLE,
	OP_GET_VARIABLE,
	OP_POP,

	OP_ENTER_SCOPE,
	OP_EXIT_SCOPE,

	OP_PRINT,
	OP_JUMP,
	OP_JUMP_IF_FALSE,
} OpCode;

typedef struct {
	Location* op_locations;
	byte_t* code;
	size_t size, capacity;
} Chunk;
void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, byte_t byte, Location location);
void freeChunk(Chunk* chunk);

void disassembleChunk(Chunk* chunk, const char* name);
void serializeChunk(Chunk* chunk, const char* filename);
#endif

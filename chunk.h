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
#endif

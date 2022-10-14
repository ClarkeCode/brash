#ifndef BRASH_CHUNK
#define BRASH_CHUNK
#include <stddef.h>
#include <stdint.h>
#include "value.h"
#include "lexer.h" //Location
typedef uint8_t byte_t;
typedef enum {
	OP_RETURN,
	OP_CONSTANT,
	OP_NEGATE,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_MODULO,
} OpCode;

typedef struct {
	Location* op_locations;
	byte_t* code;
	size_t size, capacity;

	Value* constants;
	size_t c_size, c_cap;
} Chunk;
void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, byte_t byte, Location location);
void freeChunk(Chunk* chunk);
size_t recordConstant(Chunk* chunk, Value value);
void disassembleChunk(Chunk* chunk, const char* name);
#endif

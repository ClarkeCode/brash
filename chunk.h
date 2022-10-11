#ifndef BRASH_CHUNK
#define BRASH_CHUNK
#include <stddef.h>
#include <stdint.h>
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
	byte_t* code;
	size_t size;
	size_t capacity;
} Chunk;
void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, byte_t byte);
void freeChunk(Chunk* chunk);
void disassembleChunk(Chunk* chunk, const char* name);
#endif

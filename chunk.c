#include <stdio.h>
#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
	chunk->size = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
}
void writeChunk(Chunk* chunk, byte_t byte) {
	if (chunk->size >= chunk->capacity) {
		size_t oldCapacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(oldCapacity);
		chunk->code = GROW_ARRAY(byte_t, chunk->code, oldCapacity, chunk->capacity);
	}
	
	chunk->code[chunk->size++] = byte;
}
void freeChunk(Chunk* chunk) {
	FREE_ARRAY(byte_t, chunk->code, chunk->capacity);
	initChunk(chunk);
}

//For compiler debug/output
#include "enum_lookups.h"
size_t disassembleInstruction(Chunk* chunk, size_t offset) {
	printf("%04d ", offset);
	byte_t instruction = chunk->code[offset];
	switch (instruction) {
		case OP_RETURN:
			printf("%s\n", getStr_OpCode(OP_RETURN));
			return offset + 1;
		default:
			printf("Unknown opcode %d\n", instruction);
			return offset + 1;
	}
}
void disassembleChunk(Chunk* chunk, const char* name) {
	if (name)
		printf("== %s ==\n", name);
	else
		printf("== UNNAMED_CHUNK ==\n");
	for (size_t offset = 0; offset < chunk->size;) {
		offset = disassembleInstruction(chunk, offset);
	}
}

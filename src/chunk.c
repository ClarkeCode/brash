#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
	chunk->code = NULL;
	chunk->op_locations = NULL;
	chunk->size = 0;
	chunk->capacity = 0;
}
void writeChunk(Chunk* chunk, byte_t byte, Location location) {
	if (chunk->size >= chunk->capacity) {
		size_t oldCapacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(oldCapacity);
		chunk->code = GROW_ARRAY(byte_t, chunk->code, oldCapacity, chunk->capacity);
		chunk->op_locations = GROW_ARRAY(Location, chunk->op_locations, oldCapacity, chunk->capacity);
	}
	
	chunk->code[chunk->size] = byte;
	chunk->op_locations[chunk->size] = location;
	chunk->size++;
}
void freeChunk(Chunk* chunk) {
	FREE_ARRAY(byte_t, chunk->code, chunk->capacity);
	FREE_ARRAY(byte_t, chunk->op_locations, chunk->capacity);
	initChunk(chunk);
}

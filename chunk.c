#include <stdio.h>
#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
	chunk->code = NULL;
	chunk->op_locations = NULL;
	chunk->size = 0;
	chunk->capacity = 0;

	chunk->constants = NULL;
	chunk->c_size = 0;
	chunk->c_cap = 0;
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
	FREE_ARRAY(Value, chunk->constants, chunk->c_cap);
	initChunk(chunk);
}
size_t recordConstant(Chunk* chunk, Value value) {
	if (chunk->c_size >= chunk->c_cap) {
		size_t oldCapacity = chunk->c_cap;
		chunk->c_cap = GROW_CAPACITY(oldCapacity);
		chunk->constants = GROW_ARRAY(Value, chunk->constants, oldCapacity, chunk->c_cap);
	}
	chunk->constants[chunk->c_size++] = value;
	return chunk->c_size - 1;
}

//For compiler debug/output
#define outfile stdout
#include "enum_lookups.h"
void printValue(Value value) {
	if (value.type == VAL_NUMBER)
		fprintf(outfile, "<%s : %f>", getStr_value_t(value.type), value.as.number);
	if (value.type == VAL_BOOLEAN)
		fprintf(outfile, "<%s : %s>", getStr_value_t(value.type), (value.as.boolean) ? "true" : "false");
}
#define PRINT_SINGLE_BYTE(op) fprintf(outfile, "%s\n", getStr_OpCode(op)); return offset + 1
size_t disassembleInstruction(Chunk* chunk, size_t offset) {
	printf("%04d ", offset);
	byte_t instruction = chunk->code[offset];
	switch (instruction) {
		case OP_RETURN:   PRINT_SINGLE_BYTE(OP_RETURN);
		case OP_NEGATE:   PRINT_SINGLE_BYTE(OP_NEGATE);
		case OP_ADD:      PRINT_SINGLE_BYTE(OP_ADD);
		case OP_SUBTRACT: PRINT_SINGLE_BYTE(OP_SUBTRACT);
		case OP_MULTIPLY: PRINT_SINGLE_BYTE(OP_MULTIPLY);
		case OP_DIVIDE:   PRINT_SINGLE_BYTE(OP_DIVIDE);
		case OP_MODULO:   PRINT_SINGLE_BYTE(OP_MODULO);

		case OP_CONSTANT:
			fprintf(outfile, "%-15s", getStr_OpCode(OP_CONSTANT));
			byte_t index = chunk->code[offset+1];
			printValue(chunk->constants[index]);
			fprintf(outfile, "\n");
			return offset + 2;

		default:
			fprintf(outfile, "Unknown opcode %d\n", instruction);
			return offset + 1;
	}
}
#undef PRINT_SINGLE_BYTE
#undef outfile

void disassembleChunk(Chunk* chunk, const char* name) {
	if (name)
		printf("== %s ==\n", name);
	else
		printf("== UNNAMED_CHUNK ==\n");
	for (size_t offset = 0; offset < chunk->size;) {
		offset = disassembleInstruction(chunk, offset);
	}
}

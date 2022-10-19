#include <stdio.h>
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

#include <string.h>
#include "enum_lookups.h"
#define outfile stdout
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
		case OP_EQUALS:   PRINT_SINGLE_BYTE(OP_EQUALS);
		case OP_GREATER:  PRINT_SINGLE_BYTE(OP_GREATER);
		case OP_LESSER:   PRINT_SINGLE_BYTE(OP_LESSER);

		case OP_TRUE:     PRINT_SINGLE_BYTE(OP_TRUE);
		case OP_FALSE:    PRINT_SINGLE_BYTE(OP_FALSE);
		case OP_NOT:      PRINT_SINGLE_BYTE(OP_NOT);
		case OP_AND:      PRINT_SINGLE_BYTE(OP_AND);
		case OP_OR:       PRINT_SINGLE_BYTE(OP_OR);
		case OP_XOR:      PRINT_SINGLE_BYTE(OP_XOR);

		case OP_NUMBER:
			fprintf(outfile, "%-15s", getStr_OpCode(OP_NUMBER));
			fprintf(outfile, "%f", readDoubleFromBytes(chunk->code + (offset+1)));
			fprintf(outfile, "\n");
			return offset + 1 + 8;
		case OP_STRING:// {
			fprintf(outfile, "%-15s", getStr_OpCode(instruction));
			size_t length = strlen((char*)(chunk->code + offset + 1));
			char* strcontent = malloc(sizeof(byte_t) * (length+1));
			strcpy(strcontent, (char*)(chunk->code + offset + 1));
			fprintf(outfile, " '%s'\n", strcontent);
			free(strcontent);
			return offset + 1 + length + 1;
			//} break;

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

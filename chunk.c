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
#define ONEBYTE_FMT "%-18s"
#define PRINT_SINGLE_BYTE(op) fprintf(outfile, "%s\n", getStr_OpCode(op)); return offset + 1
char* makeStringFromBytes(byte_t* bytes) {
	size_t length = strlen((char*) bytes);
	char* strcontent = malloc(sizeof(byte_t) * (length + 1));
	strcpy(strcontent, (char*)bytes);
	return strcontent;
}
size_t disassembleInstruction(Chunk* chunk, size_t offset) {
	printf("%04d ", offset);
	byte_t instruction = chunk->code[offset];
	switch (instruction) {
		//Single-byte instructions
		case OP_RETURN:
		case OP_POP:
		case OP_NEGATE:
		case OP_ADD:
		case OP_SUBTRACT:
		case OP_MULTIPLY:
		case OP_DIVIDE:
		case OP_MODULO:
		case OP_EQUALS:
		case OP_GREATER:
		case OP_LESSER:
		case OP_TRUE:
		case OP_FALSE:
		case OP_NOT:
		case OP_AND:
		case OP_OR:
		case OP_XOR:
		case OP_ENTER_SCOPE:
		case OP_EXIT_SCOPE:
		case OP_PRINT:
			PRINT_SINGLE_BYTE(instruction);

		//Multi-byte instructions
		case OP_NUMBER:
			fprintf(outfile, ONEBYTE_FMT , getStr_OpCode(instruction));
			fprintf(outfile, "%f\n", readDoubleFromBytes(chunk->code + (offset+1)));
			return offset + 1 + 8;
		case OP_STRING: {
			fprintf(outfile, ONEBYTE_FMT, getStr_OpCode(instruction));
			char* strcontent = makeStringFromBytes(chunk->code + offset + 1);
			size_t length = strlen(strcontent);
			fprintf(outfile, "'%s'\n", strcontent);
			free(strcontent);
			return offset + 1 + length + 1;
			} break;
		case OP_SET_VARIABLE: {
			fprintf(outfile, ONEBYTE_FMT, getStr_OpCode(instruction));
			char* strcontent = makeStringFromBytes(chunk->code + offset + 1);
			size_t lenth = strlen(strcontent);
			fprintf(outfile, "'%s'\n", strcontent);
			free(strcontent);
			return offset + 1 + lenth + 1;
			} break;
		case OP_GET_VARIABLE: {
			fprintf(outfile, ONEBYTE_FMT, getStr_OpCode(instruction));
			char* strcontent = makeStringFromBytes(chunk->code + offset + 1);
			size_t lenth = strlen(strcontent);
			fprintf(outfile, "'%s'\n", strcontent);
			free(strcontent);
			return offset + 1 + lenth + 1;
			} break;

		case OP_JUMP:
		case OP_JUMP_IF_FALSE: {
			fprintf(outfile, ONEBYTE_FMT, getStr_OpCode(instruction));
			int16_t jumpRelative = readInt16FromBytes(chunk->code + offset + 1);
			fprintf(outfile, "%+d (goto: %d)\n", jumpRelative, offset + jumpRelative);
			return offset + 3;
			} break;
		case OP_LOOP: {
			fprintf(outfile, ONEBYTE_FMT, getStr_OpCode(instruction));
			int16_t jumpRelative = readInt16FromBytes(chunk->code + offset + 1);
			fprintf(outfile, "%+d (goto: %d)\n", jumpRelative, offset - jumpRelative);
			return offset + 3;
			} break;

		default:
			fprintf(outfile, "Unknown opcode %d '%c'\n", instruction, instruction);
			return offset + 1;
	}
}
#undef PRINT_SINGLE_BYTE
#undef ONEBYTE_FMT
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
void serializeChunk(Chunk* chunk, const char* filename) {
	FILE* outfile = fopen(filename, "wb");
	for (size_t x = 0; x < chunk->size; x++) {
		putc((char)chunk->code[x], outfile);
	}
	fclose(outfile);
}

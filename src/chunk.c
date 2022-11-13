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

//bytes acts as an outparam, moving the input pointer via double indirection
char* makeStringFromBytes(byte_t** bytes) {
	size_t length = strlen((char*) *bytes);
	char* strcontent = malloc(sizeof(byte_t) * (length + 1));
	strcpy(strcontent, (char*)(*bytes));
	(*bytes) += length + 1;
	return strcontent;
}

byte_t* disassembleInstruction(byte_t* start, byte_t* current) {
	size_t offset = current - start;
	byte_t instruction = *current;
	printf("%04d ", offset);
	current += 1; //Consume instruction OpCode
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
			fprintf(outfile, "%s\n", getStr_OpCode(instruction));
			return current;

		//Multi-byte instructions
		case OP_NUMBER:
			fprintf(outfile, ONEBYTE_FMT , getStr_OpCode(instruction));
			fprintf(outfile, "%f\n", readDoubleFromBytes(current));
			return current + 8;

		case OP_STRING:
			fprintf(outfile, ONEBYTE_FMT , getStr_OpCode(instruction));
			{
			char* strcontent = makeStringFromBytes(&current);
			fprintf(outfile, "'%s'\n", strcontent);
			free(strcontent);
			}
			return current;

		case OP_DEF_VARIABLE:
			fprintf(outfile, ONEBYTE_FMT, getStr_OpCode(instruction));
			{
			char* strcontent = makeStringFromBytes(&current);
			fprintf(outfile, "'%s'\n", strcontent);
			free(strcontent);
			}
			return current;

		case OP_SET_VARIABLE:
			fprintf(outfile, ONEBYTE_FMT, getStr_OpCode(instruction));
			{
			char* strcontent = makeStringFromBytes(&current);
			fprintf(outfile, "'%s'\n", strcontent);
			free(strcontent);
			}
			return current;

		case OP_GET_VARIABLE:
			fprintf(outfile, ONEBYTE_FMT, getStr_OpCode(instruction));
			{
			char* strcontent = makeStringFromBytes(&current);
			fprintf(outfile, "'%s'\n", strcontent);
			free(strcontent);
			}
			return current;

		case OP_DEC_FUNCTION:
			fprintf(outfile, ONEBYTE_FMT , getStr_OpCode(instruction));
			{
			char* strcontent = makeStringFromBytes(&current);
			fprintf(outfile, "\n\tName: '%s' ", strcontent);
			free(strcontent);
			}

			//Parameters
			uint8_t arity = (uint8_t) *current;
			current += 1;
			Types paramTypes[255] = {0};
			char* paramNames[255];

			for (uint8_t x = 0; x < arity; x++) {
				paramTypes[x] = (Types) *current;
				current += 1;
			}
			for (uint8_t x = 0; x < arity; x++) {
				char* paramName = makeStringFromBytes(&current);
				paramNames[x] = paramName;
			}

			//Returns
			uint8_t rarity = (uint8_t) *current;
			current += 1;
			Types returnTypes[255] = {0};
			for (uint8_t x = 0; x < rarity; x++) {
				returnTypes[x] = (Types) *current;
				current += 1;
			}


			fprintf(outfile, "Parameters: %d ", arity);
			fprintf(outfile, "Returns: %d\n", rarity);

			for (uint8_t x = 0; x < arity; x++) {
				fprintf(outfile, "\tParam #%d %s %s\n", x, getStr_Types(paramTypes[x]), paramNames[x]);
				if (paramNames[x]) free(paramNames[x]);
			}
			for (uint8_t x = 0; x < rarity; x++) {
				fprintf(outfile, "\tReturn #%d %s\n", x, getStr_Types(returnTypes[x]));
			}
			return current;

		case OP_FUNCTION_CALL:
			fprintf(outfile, ONEBYTE_FMT, getStr_OpCode(instruction));
			{
			char* strcontent = makeStringFromBytes(&current);
			fprintf(outfile, "'%s'\n", strcontent);
			free(strcontent);
			}
			return current;

		case OP_JUMP:
		case OP_JUMP_IF_FALSE:
			fprintf(outfile, ONEBYTE_FMT, getStr_OpCode(instruction));
			{
			int16_t jumpRelative = readInt16FromBytes(current);
			fprintf(outfile, "%+d (goto: %d)\n", jumpRelative, offset + jumpRelative);
			}
			return current + 2;
		case OP_LOOP:
			fprintf(outfile, ONEBYTE_FMT, getStr_OpCode(instruction));
			{
			int16_t jumpRelative = readInt16FromBytes(current);
			fprintf(outfile, "%+d (goto: %d)\n", jumpRelative, offset - jumpRelative);
			}
			return current + 2;

		default:
			fprintf(outfile, "Unknown opcode %d '%c'\n", instruction, instruction);
			return current;
	}
}
#undef ONEBYTE_FMT
#undef outfile

void disassembleChunk(Chunk* chunk, const char* name) {
	byte_t* current = chunk->code;
	byte_t* end = chunk->code + chunk->size;
	while (current < end) {
		current = disassembleInstruction(chunk->code, current);
	}
}
void serializeChunk(Chunk* chunk, const char* filename) {
	FILE* outfile = fopen(filename, "wb");
	for (size_t x = 0; x < chunk->size; x++) {
		putc((char)chunk->code[x], outfile);
	}
	fclose(outfile);
}

#include <stdlib.h>
#include "chunk.h"
#include <stdio.h>
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

		case OP_DEC_FUNCTION: {
			BrashFunc func = {0};
			decodeFunc(current, &func);

			fprintf(outfile, ONEBYTE_FMT , getStr_OpCode(instruction));
			fprintf(outfile, "\n\tName: '%s' ", func.name);
			fprintf(outfile, "Parameters: %d ", func.arity);
			fprintf(outfile, "Returns: %d ", func.rarity);

			size_t resumeOffset = (func.afterBody - start);
			fprintf(outfile, "Width: (%d)\n\t(func end: %d)\n", resumeOffset-offset-1, resumeOffset);

			for (uint8_t x = 0; x < func.arity; x++) {
				fprintf(outfile, "\tParam #%d %s %s\n", x, getStr_Types(func.paramTypes[x]), func.paramNames[x]);
			}
			for (uint8_t x = 0; x < func.rarity; x++) {
				fprintf(outfile, "\tReturn #%d %s\n", x, getStr_Types(func.returnTypes[x]));
			}
			return func.functionStart;
		}

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
			uint16_t jumpRelative = readUInt16FromBytes(current);
			fprintf(outfile, "%+d (goto: %d)\n", jumpRelative, offset + jumpRelative);
			}
			return current + 2;
		case OP_LOOP:
			fprintf(outfile, ONEBYTE_FMT, getStr_OpCode(instruction));
			{
			uint16_t jumpRelative = readUInt16FromBytes(current);
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

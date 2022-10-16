#ifndef BRASH_DEBUGGING
#define BRASH_DEBUGGING
#include <stdio.h>

typedef enum {
	LEX_TOKEN_PRODUCTION = 1u << 1,
	COM_BYTE_EMISSION    = 1u << 2,
	VM_READING_INSTRUCTIONS = 1u << 3,
} Flags;

void setDebugFlags(Flags flags);

void printDebug(FILE* fp, Flags whenToPrint, const char* format, ...);
#endif

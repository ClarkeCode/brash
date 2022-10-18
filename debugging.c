#include <stdio.h>
#include <stdarg.h>
#include "debugging.h"

Flags dbflags = (Flags)0;

void printDebug(FILE* fp, Flags whenToPrint, const char* format, ...) {
#ifdef BRASH_ALLOW_DEBUG_OUTPUT
	if (dbflags & whenToPrint) {
		va_list args;
		va_start(args, format);
		vfprintf(fp, format, args);
		va_end(args);
	}
#endif
}

void setDebugFlags(Flags flags) {
#ifdef BRASH_ALLOW_DEBUG_OUTPUT
	dbflags = flags;
#endif
}

int hasDebugFlag(Flags flag) {
#ifdef BRASH_ALLOW_DEBUG_OUTPUT
	return dbflags & flag;
#else
	return 0;
#endif
}

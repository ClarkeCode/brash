#ifndef BRASH_COMPILER
#define BRASH_COMPILER
#include <stdbool.h>
#include "chunk.h"
bool compile(const char* source, Chunk* chunk);
#endif

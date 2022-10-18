#ifndef BRASH_BYTES
#define BRASH_BYTES
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t byte_t;

void transpose8Bytes(void* sourceBuffer, void* outBuffer);
double readDoubleFromBytes(void* sourceBuffer);
#endif

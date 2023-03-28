#ifndef BRASH_MEMORY
#define BRASH_MEMORY
#include <stdlib.h>

void* reallocate(void* pointer, size_t oldCap, size_t newCap);
#define GROW_CAPACITY(capacity)\
	((capacity) < 8 ? 8 : (capacity)*2)
#define GROW_ARRAY(type, pointer, oldCap, newCap)\
	(type*) reallocate(pointer, sizeof(type)*(oldCap), sizeof(type)*(newCap))
#define FREE_ARRAY(type, pointer, oldCap)\
	reallocate(pointer, sizeof(type)*oldCap, 0)

#endif

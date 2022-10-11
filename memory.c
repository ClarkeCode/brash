#include "memory.h"
#include "virtualmachine.h"

void* reallocate(void* pointer, size_t oldCap, size_t newCap) {
	vm.allocatedBytes += (newCap - oldCap);

	if (newCap == 0) {
		free(pointer);
		return NULL;
	}

	void* result = realloc(pointer, newCap);
	if (result == NULL) exit(1);
	return result;
}

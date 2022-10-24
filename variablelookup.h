#ifndef BRASH_VARIABLE_LOOKUP
#define BRASH_VARIABLE_LOOKUP
#include "value.h"

typedef struct {
	char** strings;
	size_t size;
	size_t capacity;
} StringInternment;

typedef struct {
	char** _names;
	size_t _name_capacity;
	size_t _internal_size;

	Value* _values;
	StringInternment* _internment;
} VariableLookup;


VariableLookup* make_variable_lookup();
void free_variable_lookup(VariableLookup* lookup);
void lookup_add(VariableLookup* lookup, char* name, Value val);
Value lookup_get(VariableLookup* lookup, char* lookupkey);
bool lookup_has(VariableLookup* lookup, char* lookupkey);
#endif

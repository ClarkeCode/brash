#ifndef BRASH_VARIABLE_LOOKUP
#define BRASH_VARIABLE_LOOKUP
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "structs.h"


VariableLookup* make_variable_lookup();
void free_variable_lookup(VariableLookup* lookup);
void lookup_add(VariableLookup* lookup, char* name, Value val);
Value lookup_get(VariableLookup* lookup, char* lookupkey);
bool lookup_has(VariableLookup* lookup, char* lookupkey);
void dump_variable_lookup(FILE* fp, VariableLookup* lookup);
#endif

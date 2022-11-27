#include <stdlib.h>
#include <string.h>
#include "variablelookup.h"

char* _newcpy(char* tocopy) {
	char* nstr = malloc(strlen(tocopy)+1);
	strcpy(nstr, tocopy);
	return nstr;
}

VariableLookup* make_variable_lookup() {
	VariableLookup* lookup = (VariableLookup*) calloc(1, sizeof(VariableLookup));
	lookup->_name_capacity = 2;
	lookup->_internal_size = 0;
	lookup->_names = (char**) malloc(lookup->_name_capacity * sizeof(char*));
	lookup->_values = (Value*) malloc(lookup->_name_capacity * sizeof(Value));
	return lookup;
}
void free_variable_lookup(VariableLookup* lookup) {
	if (!lookup) return;
	for (size_t x = 0; x < lookup->_internal_size; x++) {
		if (lookup->_names[x])
			free(lookup->_names[x]);
	}
	if (lookup->_values) free(lookup->_values);
	if (lookup->_names) free(lookup->_names);
	if (lookup) free(lookup);
}

void lookup_add(VariableLookup* lookup, char* name, Value val) {
	if (lookup->_internal_size == lookup->_name_capacity) {
		//resize
		lookup->_name_capacity *= 2;
		char** oldptr = lookup->_names;
		lookup->_names = realloc(lookup->_names, lookup->_name_capacity * sizeof(char*));
		if (!lookup->_names) { //Realloc failure
			lookup->_names = oldptr;
			fprintf(stderr, "Realloc failure for names");
			exit(EXIT_FAILURE);
		}

		Value* oldval = lookup->_values;
		lookup->_values = realloc(lookup->_values, lookup->_name_capacity * sizeof(Value));
		if (!lookup->_values) { //Realloc failure
			lookup->_values = oldval;
			fprintf(stderr, "Realloc failure for values");
			exit(EXIT_FAILURE);
		}
	}

	if (lookup_has(lookup, name)) {
		size_t index = 0;
		for (; index < lookup->_internal_size; index++) {
			if (strcmp(lookup->_names[index], name) == 0) break;
		}
		lookup->_values[index] = val;
	}
	else {
		lookup->_names[lookup->_internal_size] = _newcpy(name);
		Value* nval = lookup->_values + lookup->_internal_size;
		*nval = val; //Copy value
		lookup->_internal_size++;
	}
}

Value lookup_get(VariableLookup* lookup, char* lookupkey) {
	for (size_t x = 0; x < lookup->_internal_size; x++) {
		if (strcmp(lookup->_names[x], lookupkey) == 0)
			return *(lookup->_values + x);
	}
	return (Value) {0};
}

bool lookup_has(VariableLookup* lookup, char* lookupkey) {
	for (size_t x = 0; x < lookup->_internal_size; x++) {
		if (strcmp(lookup->_names[x], lookupkey) == 0)
			return true;
	}
	return false;
}


#include <stdlib.h>
#include <string.h>
#include "variablelookup.h"
#include "structs.h"
#include "enumerations.h"
#include "interpreter.h"

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
		if (lookup->_names[x]) free(lookup->_names[x]);
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

	lookup->_names[lookup->_internal_size] = malloc(strlen(name) * sizeof(char) + 1);
	strcpy(lookup->_names[lookup->_internal_size], name);
	lookup->_values[lookup->_internal_size] = val; //Copy value
	lookup->_internal_size++;
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

void dump_variable_lookup(FILE* fp, VariableLookup* lookup) {
	fprintf(fp, "-- Symbol lookup --\n");
	for (size_t x = 0; x < lookup->_internal_size; x++) {
		fprintf(fp, "%-10s ", lookup->_names[x]);
		dump_value(fp, lookup->_values + x);
		fprintf(fp, "\n");
	}
}

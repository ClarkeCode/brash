#include <stdlib.h>
#include <string.h>
#include "variablelookup.h"
#include "structs.h"
#include "enumerations.h"
#include "interpreter.h"

StringInternment* make_string_internment() {
	StringInternment* internment = (StringInternment*) malloc(sizeof(StringInternment));
	internment->size = 0;
	internment->capacity = 10;
	internment->strings = (char**) calloc(internment->capacity, sizeof(char*));
	return internment;
}
void free_string_internment(StringInternment* si) {
	if (!si) return;
	for (size_t x = 0; x < si->size; x++) {
		if (si->strings[x]) free(si->strings[x]);
	}
	if (si->strings) free(si->strings);
	if (si) free(si);
}
char* intern_string(VariableLookup* lookup, char* string) {
	StringInternment* intern = lookup->_internment;
	if (intern->size == intern->capacity) {
		intern->capacity *= 2;
	}
	size_t x = 0;
	for (; x < intern->size; x++) {
		if (strcmp(string, intern->strings[x]) == 0) {
			return intern->strings[x];
		}
	}
	
	intern->strings[x] = (char*) calloc(strlen(string)+1, sizeof(char));
	strcpy(intern->strings[x], string);
	intern->size++;
	return intern->strings[x];
}

VariableLookup* make_variable_lookup() {
	VariableLookup* lookup = (VariableLookup*) calloc(1, sizeof(VariableLookup));
	lookup->_internment = (StringInternment*) make_string_internment();
	lookup->_name_capacity = 2;
	lookup->_internal_size = 0;
	lookup->_names = (char**) malloc(lookup->_name_capacity * sizeof(char*));
	lookup->_values = (Value*) malloc(lookup->_name_capacity * sizeof(Value));
	return lookup;
}
void free_variable_lookup(VariableLookup* lookup) {
	if (!lookup) return;
	free_string_internment(lookup->_internment);
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
		if (lookup->_values[index].type == TYPE_STRING) {
			lookup->_values[index].as.string = intern_string(lookup, lookup->_values[index].as.string);
		}
	}
	else {
		lookup->_names[lookup->_internal_size] = intern_string(lookup, name);
		Value* nval = lookup->_values + lookup->_internal_size;
		*nval = val; //Copy value
		if (nval->type == TYPE_STRING) {
			nval->as.string = intern_string(lookup, val.as.string);
		}
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


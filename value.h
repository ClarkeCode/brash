#ifndef BRASH_VALUES
#define BRASH_VALUES
#include <stddef.h>
#include <stdbool.h>

typedef enum {
	VAL_NOT_VALUE,
	VAL_BOOLEAN,
	VAL_NUMBER,
	VAL_OBJECT
} value_t;

typedef enum {
	OBJ_STRING,
} obj_t;

typedef struct _object {
	obj_t type;
	struct _object* next;
} Object;

typedef struct {
	value_t type;
	union {
		bool boolean;
		double number;
		Object* object;
	} as;
} Value;

typedef struct {
	Object obj;
	size_t len;
	char* cstr;
} ObjectString;

bool valueEquality(Value v1, Value v2);

ObjectString* makeString(char* bytes);
void freeObject(Object* obj);

#include <stdio.h>
void printValue(FILE* outfile, Value value);
#endif

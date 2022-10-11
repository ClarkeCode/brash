#include <string.h>
#include "value.h"


bool _objectValueEquality(Value v1, Value v2) {
	if ((v1.as.object)->type != (v2.as.object)->type) return false;
	switch ((v1.as.object)->type) {
		case OBJ_STRING: {
			ObjectString* osv1 = (ObjectString*)v1.as.object;
			ObjectString* osv2 = (ObjectString*)v2.as.object;
			if (osv1->len != osv2->len) return false;
			return strcmp(osv1->cstr, osv2->cstr) == 0;
		} break;
	}
	return false;
}
bool valueEquality(Value v1, Value v2) {
	if (v1.type != v2.type) return false;

	if (v1.type == VAL_BOOLEAN) return !(v1.as.boolean ^ v2.as.boolean);
	if (v1.type == VAL_NUMBER) return v1.as.number == v2.as.number;
	if (v1.type == VAL_OBJECT) {
		return _objectValueEquality(v1, v2);
	}

	return false;
}


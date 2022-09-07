typedef enum {
	TYPE_NOT_VALUE,
	TYPE_BOOL,
	TYPE_NUMBER,
	TYPE_STRING
} value_t;

typedef struct {
	value_t type;
	union {
		bool boolean;
		double number;
		char* string;
	} as;
} Value;

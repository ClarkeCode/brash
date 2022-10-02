#ifndef SIMPLE_COMMAND_LINE
#define SIMPLE_COMMAND_LINE
//Version: 1.0.0
#include <stddef.h>
#include <stdbool.h>

typedef struct {
	size_t index;
	size_t size;

	size_t cap_memory;
	char** records;
} StrHistory;

typedef struct {
	char* name;
	size_t headlength;

	size_t lineCapacity;
	char* candidate;
	char* stMemory;
	size_t cursorIndex;

	bool isViewingHistory;
	StrHistory* history;
} CmdLine;

void assignCommandText(CmdLine* line, char* name);

CmdLine* make_cmdline(char* name, size_t desiredHistoryLength);

void free_cmdline(CmdLine* line);

//Note: returned string is heap allocated; must be freed by caller
char* grabInputLine(CmdLine* line);
#endif

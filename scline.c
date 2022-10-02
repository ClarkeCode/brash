//https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html
#include "scline.h"

#include <unistd.h> //read(), STDIN_FILENO
#include <termios.h>
#include <stdlib.h> //atexit()
#include <errno.h> //perror(), errno, EAGAIN

#include <stdio.h> //printf(), perror()
#include <ctype.h>
#include <string.h> //memset()
#include <stdbool.h>

struct termios orig_termios;

void die(const char* message) {
	perror(message);
	exit(EXIT_FAILURE);
}

void disableRawMode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
		die("tcsetattr");
}

void enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
	atexit(disableRawMode);
	
	struct termios raw = orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); //Disable ctl S,Q ; disable \r translation
	raw.c_oflag &= ~(OPOST); //Disable automatic return carriage on newline out
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON /*| ISIG*/); //Disable ctl V

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

void* perform_realloc(void* ptr, size_t capacity) {
	void* newptr = realloc(ptr, capacity);
	if (!newptr) exit(EXIT_FAILURE);
	return newptr;
}

#define DEFAULT_LINE_LENGTH 128

StrHistory* make_strhistory(size_t max_history) {
	StrHistory* history = (StrHistory*) calloc(1, sizeof(StrHistory));
	history->index = 0;
	history->size = 0;
	history->cap_memory = max_history;
	history->records = calloc(history->cap_memory, sizeof(char*));
	for (size_t x = 0; x < history->cap_memory; x++) {
		history->records[x] = calloc(DEFAULT_LINE_LENGTH, sizeof(char));
	}
	return history;
}

void free_strhistory(StrHistory* hist) {
	if (!hist) return;
	for (size_t x = 0; x < hist->cap_memory; x++) {
		if (hist->records[x]) free(hist->records[x]);
	}
	if (hist->records) free(hist->records);
	if (hist) free(hist);
}

void _historyRecordLine(StrHistory* hist, char* addition) {
	if (strcmp(addition, hist->records[0]) == 0 ||
		strlen(addition) == 0) return; //Do nothing if addition is the same as first record or blank
	for (size_t x = hist->cap_memory-1; x >= 1; x--) {
		strcpy(hist->records[x], hist->records[x-1]);
	}
	strcpy(hist->records[0], addition);
	if (hist->size < hist->cap_memory) hist->size++;
}

void _historyWidenLineCapacity(StrHistory* hist, size_t newWidth) {
	for (size_t x = 0; x < hist->cap_memory; x++) {
		hist->records[x] = (char*) perform_realloc(hist->records[x], newWidth);
	}
}

void _historyMoveLater(StrHistory* history) {
	if (history->size > 0 &&
		history->index+1 < history->size &&
		history->index+1 < history->cap_memory) history->index++;
}
void _historyMoveSooner(StrHistory* history) {
	if (history->size > 0) history->index--;
}
void _historyMoveToLast(StrHistory* history) {
	if (history->size > 0) history->index = history->size-1;
}
void _historyMoveToFirst(StrHistory* history) {
	history->index = 0;
}
char* _historyGetAtIndex(StrHistory* history) {
	if (history->size == 0) return NULL;
	return history->records[history->index];
}

void assignCommandText(CmdLine* line, char* name) {
	line->name = name;
	line->headlength = strlen(name);
}

CmdLine* make_cmdline(char* name, size_t desiredHistoryLength) {
	CmdLine* line = (CmdLine*) malloc(sizeof(CmdLine));
	assignCommandText(line, name);

	line->lineCapacity = DEFAULT_LINE_LENGTH;
	line->cursorIndex = 0;
	line->candidate = (char*) calloc(line->lineCapacity, sizeof(char));
	line->stMemory = (char*) calloc(line->lineCapacity, sizeof(char));

	line->isViewingHistory = false;

	line->history = make_strhistory(desiredHistoryLength);
	return line;
}
void free_cmdline(CmdLine* line) {
	if (!line) return;
	free_strhistory(line->history);
	if (line->candidate) free(line->candidate);
	if (line->stMemory) free(line->stMemory);
	if (line) free(line);
}
void _insertAtCandidateCursor(CmdLine* line, char c) {
	//If the next character would exceed the current capacity, reallocate candidate and stMemory to be larger
	if (line->cursorIndex+1 >= line->lineCapacity) {
		size_t newCapacity = line->lineCapacity * 2;
		line->candidate = (char*) perform_realloc(line->candidate, newCapacity);
		line->stMemory = (char*) perform_realloc(line->stMemory, newCapacity);
		//Set all newly allocated bytes to \0 so candidate is always a valid C-string, required because candidate is written incrementally
		memset(line->candidate + line->lineCapacity, 0, newCapacity - line->lineCapacity);
		line->lineCapacity = newCapacity;
		_historyWidenLineCapacity(line->history, newCapacity);
	}

	if (line->cursorIndex == strlen(line->candidate))
		line->candidate[line->cursorIndex++] = c;
	else {
		//shift to right
		for (size_t x = strlen(line->candidate); x > line->cursorIndex+1; x--)
			line->candidate[x] = line->candidate[x-1];
		line->candidate[line->cursorIndex++] = c;
	}
}
void processVisibleChar(CmdLine* line, char c) {
	if (line->isViewingHistory) {
		line->isViewingHistory = false;
		_historyMoveToFirst(line->history);
	}
	_insertAtCandidateCursor(line, c);
}
void showCmdLine(CmdLine* line) {
	//See https://vt100.net/docs/vt100-ug/chapter3.html
	//\r\x1b[0j  Clear everything on the current line
	//%s%s       Write the command line and the current candidate
	//\r\x1b[%C  Return to line start and move cursor % cells to the right
	//\x1b[1A\n  Move cursor one position up, then newline
	printf("\r\x1b[0J%s%s\r\x1b[%dC\x1b[1A\n",
			line->name, line->candidate,
			(int)(line->headlength + line->cursorIndex)
		);
}
enum specialKeys {
	UNKNOWN,
	ARROW_LEFT,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,

	PAGE_UP,
	PAGE_DOWN,
	HOME,
	END,
	INSERT,
	DELETE,

	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	ESC_KEY,
};
enum specialKeys decodeEscapeSequence() {
	char seq[4];
	if (read(STDIN_FILENO, &seq[0], 1) != 1) return ESC_KEY;
	if (read(STDIN_FILENO, &seq[1], 1) != 1) return ESC_KEY;

	if (seq[0] == 'O') {
		if (seq[1] == 'P') return KEY_F1;
		if (seq[1] == 'Q') return KEY_F2;
		if (seq[1] == 'R') return KEY_F3;
		if (seq[1] == 'S') return KEY_F4;
	}
	if (seq[0] == '[') {
		if (seq[1] >= '0' && seq[1] <= '9') {
			if (read(STDIN_FILENO, &seq[2], 1) != 1) return UNKNOWN;
			if (seq[2] == '~' && seq[1] == '2') return INSERT;
			if (seq[2] == '~' && seq[1] == '3') return DELETE;
			if (seq[2] == '~' && seq[1] == '5') return PAGE_UP;
			if (seq[2] == '~' && seq[1] == '6') return PAGE_DOWN;
			if (seq[2] == '~' && seq[1] == '7') return HOME;
			if (seq[2] == '~' && seq[1] == '8') return END;

			if (read(STDIN_FILENO, &seq[3], 1) != 1) return UNKNOWN;
			if (seq[1] == '1' && seq[2] == '5' && seq[3] == '~') return KEY_F5;
			if (seq[1] == '1' && seq[2] == '7' && seq[3] == '~') return KEY_F6;
			if (seq[1] == '1' && seq[2] == '8' && seq[3] == '~') return KEY_F7;
			if (seq[1] == '1' && seq[2] == '9' && seq[3] == '~') return KEY_F8;
			if (seq[1] == '2' && seq[2] == '0' && seq[3] == '~') return KEY_F9;
			if (seq[1] == '2' && seq[2] == '1' && seq[3] == '~') return KEY_F10;
			if (seq[1] == '2' && seq[2] == '2' && seq[3] == '~') return KEY_F11;
			if (seq[1] == '2' && seq[2] == '3' && seq[3] == '~') return KEY_F12;
			return UNKNOWN;
		}
		if (seq[1] == 'A') return ARROW_UP;
		if (seq[1] == 'B') return ARROW_DOWN;
		if (seq[1] == 'C') return ARROW_RIGHT;
		if (seq[1] == 'D') return ARROW_LEFT;
		if (seq[1] == 'F') return END;
		if (seq[1] == 'H') return HOME;
	}

	return UNKNOWN;
}
void _backspaceAtCursor(CmdLine* line) {
	for (size_t x = line->cursorIndex-1; x < line->lineCapacity-1; x++) {
		line->candidate[x] = line->candidate[x+1];
	}
	if (line->cursorIndex > 0) line->cursorIndex--;
}
void _writeToCandidate(CmdLine* line, char* desiredValue) {
	memset(line->candidate, 0, line->lineCapacity);
	strcpy(line->candidate, desiredValue);
	line->cursorIndex = strlen(line->candidate);
}
void _rememberCurrentCandidate(CmdLine* line) {
	strcpy(line->stMemory, line->candidate);
}
void processCommandSequence(CmdLine* line, char c) {
	if (c == 127 && line->cursorIndex > 0) { //Backspace
		if (line->cursorIndex == strlen(line->candidate))
			line->candidate[--line->cursorIndex] = '\0';
		else {
			_backspaceAtCursor(line);
		}
	}

	if (c == '\x1b') {
		switch (decodeEscapeSequence()) {
			case UNKNOWN:
				printf("Unknown ESC code");
				break;
			case DELETE:
				line->cursorIndex++;
				_backspaceAtCursor(line);
				break;
			case ARROW_UP:
				if (line->history->size > 0) {
					if (!line->isViewingHistory) {
						_rememberCurrentCandidate(line);
						line->isViewingHistory = true;
					}
					else {
						_historyMoveLater(line->history);
					}
					_writeToCandidate(line, _historyGetAtIndex(line->history));
				}
				break;
			case ARROW_DOWN:
				if (!line->isViewingHistory) {}
				else if (line->history->index > 0) {
					_historyMoveSooner(line->history);
					_writeToCandidate(line, _historyGetAtIndex(line->history));
				}
				else {
					_writeToCandidate(line, line->stMemory);
					line->isViewingHistory = false;
				}
				break;
			case PAGE_UP:
				if (!line->isViewingHistory)
					_rememberCurrentCandidate(line);
				line->isViewingHistory = true;
				_historyMoveToLast(line->history);
				_writeToCandidate(line, _historyGetAtIndex(line->history));
				break;
			case PAGE_DOWN:
				line->isViewingHistory = false;
				_historyMoveToFirst(line->history);
				_writeToCandidate(line, line->stMemory);
				break;
			case ARROW_RIGHT:
				if (line->cursorIndex < strlen(line->candidate)) line->cursorIndex++;
				break;
			case ARROW_LEFT:
				if (line->cursorIndex > 0) line->cursorIndex--;
				break;
			case HOME:
				line->cursorIndex = 0;
				break;
			case END:
				line->cursorIndex = strlen(line->candidate);
				break;
			default:
				{}
		}
	}
}

char* grabInputLine(CmdLine* line) {
	enableRawMode();
	char c = '\0';
	while (1) {
		showCmdLine(line);
		if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
		if (c == 4) break; //Ctrl+D / EOF
		if (!iscntrl(c))
			processVisibleChar(line, c);
		else
			processCommandSequence(line, c);
		if (c == 13) break; //Newline
	}
	char* output = (char*) calloc(strlen(line->candidate)+1, sizeof(char));
	strcpy(output, line->candidate);

	_historyRecordLine(line->history, line->candidate);
	_historyMoveToFirst(line->history);
	line->isViewingHistory = false;

	//Reset candidate
	_writeToCandidate(line, "");
	memset(line->stMemory, 0, line->lineCapacity);

	disableRawMode();
	return output;
}

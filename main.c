#include <stdio.h>
#include <stdlib.h> //size_t
#include <stdbool.h> //bool
#include <string.h>

#include "enum_lookups.h"
#include "lexer.h"
#include "scline.h"

int main(/*int argc, char* argv[]*/) {
	char* program = "11<= \"some kind of string\"12.12 \ngregsory =    \ttrue";

	setLexer("notafile", program);
	token_t tok;

	do {
		StrView content;
		Location location;
		tok = produceNextToken(&content, &location);
		char* content_str = unbox(&content);
		printf("%s '%s' %s:%d:%d\n", getStr_token_t(tok), content_str, location.file ? location.file : "N/A", location.line, location.offset);
		free(content_str);
	}
	while (tok != TOKEN_EOF);

	return 0;
}

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
	Token tok;

	do {
		tok = produceNextToken();
		char* content_str = unbox(&tok.content);
		printf("%s '%s' %s:%d:%d\n", getStr_token_t(tok.type), content_str, tok.location.file ? tok.location.file : "N/A", tok.location.line, tok.location.offset);
		free(content_str);
	}
	while (tok.type != TK_EOF);

	return 0;
}

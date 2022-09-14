#include <stdio.h>
#include <stdlib.h> //size_t
#include <stdbool.h> //bool
#include <string.h>

#include "structs.h"
#include "enumerations.h"
#include "enum_lookups.h"
#include "lexer.h"
#include "parser_ast.h"
#include "interpreter.h"

#include <assert.h>
typedef struct {
	char** _names;
	size_t _name_capacity;
	size_t _internal_size;

	Value* _values;
} VariableLookup;

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
	for (size_t x = 0; x < lookup->_internal_size; x++) {
		fprintf(fp, "%-10s ", lookup->_names[x]);
		dump_value(fp, lookup->_values + x);
		fprintf(fp, "\n");
	}
}


int main(int argc, char* argv[]) {

	char* program;
	if (argc > 1) { program = argv[1]; }
	else          { program = "greg = !false ^^ 1+2/3-9 >= 4*5 && true"; }

	fprintf(stdout, "Program: %s\n\n", program);
	
	Lexer* lex = make_lexer(program, "notfile", 0);

	size_t num_tok = 0;
	Token* tok = lexer_tokenize_all(lex, &num_tok);
	{
		fprintf(stdout, "=== LEXER ===\n");
		dump_lexer_tokens(stdout, tok, num_tok);
		fprintf(stdout, "Remaining in lexer: '%s'\n", lex->line + lex->char_offset);

		FILE* outfile = fopen("lexer.out", "wb");
		dump_lexer_tokens(outfile, tok, num_tok);
		fclose(outfile);
	}

	Parser* parser = make_parser(tok, num_tok);
	ASTNode* ast = parse_ast(parser, -1);
	{
		FILE* fp = fopen("ast.out", "w");
		fprintf(fp, "\n=== PARSER ===\n");
		dump_tree(fp, ast);
		fclose(fp);

		fprintf(stdout, "\n=== PARSER ===\n");
		dump_tree(stdout, ast);
	}

	Interpreter* terp = make_interpreter();
	{
		fprintf(stdout, "\n=== INTERPRETER ===\n");
		interpret(terp, ast, stdout);
		fprintf(stdout, "\nInterpreter value stack final state:\n");
		dump_interpreter(stdout, terp);
	}

	VariableLookup* lookup = make_variable_lookup();
	Value v1 = {TYPE_BOOLEAN, {.boolean=true}};
	Value v2 = {TYPE_NUMBER, {.number=6.22}};
	Value v3 = {TYPE_STRING, {.string="Another thing more"}};
	lookup_add(lookup, "Testing", v1);
	lookup_add(lookup, "Greggs", v2);
	lookup_add(lookup, "More", v3);

	Value v8 = lookup_get(lookup, "More");
	dump_value(stdout, &v8);
	fprintf(stdout, "\n");
	fprintf(stdout, "%d %d\n", lookup_has(lookup, "More"), lookup_has(lookup, "Morea"));

	dump_variable_lookup(stdout, lookup);
	free_variable_lookup(lookup);

	
	//Free allocated memory
	free_interpreter(terp);
	free(parser);
	free_astnode(ast);
	for (size_t x = 0; x < num_tok; x++) free((tok+x)->content);
	free(tok);
	free(lex);

	return 0;
}

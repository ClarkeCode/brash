#include <stdio.h>
#include <stdbool.h>
#include "dumpfunctions.h"
#include "enum_lookups.h"

//Write out tokens to specified file
void dump_lexer_tokens(FILE* outfile, Token* token_arr, size_t arr_sz) {
	Token* tok = token_arr;
	for (size_t x = 0; x < arr_sz; x++) {
		tok = (token_arr + x);
		fprintf(outfile,
				"%s:%d:%-5d T%-3d %-25s '%s'\n",
				tok->origin_file, tok->origin_line, tok->origin_char,
				tok->type, getStr_token_t(tok->type), tok->content
				);
	}
}

void dump_value(FILE* fp, Value* val) {
		if (val->type == TYPE_NUMBER)
			fprintf(fp, "%-10s %f", "NUMBER",  val->as.number);
		else if (val->type == TYPE_BOOLEAN)
			fprintf(fp, "%-10s %s", "BOOLEAN", val->as.boolean ? "true" : "false");
		else if (val->type == TYPE_STRING)
			fprintf(fp, "%-10s %s", "STRING",  val->as.string);
		else
			fprintf(fp, "%-10s", "NON-VALUE");
}

void dump_value_notype(FILE* fp, Value* val) {
		if (val->type == TYPE_NUMBER)
			fprintf(fp, "%f", val->as.number);
		else if (val->type == TYPE_BOOLEAN)
			fprintf(fp, "%s", val->as.boolean ? "true" : "false");
		else if (val->type == TYPE_STRING)
			fprintf(fp, "%s", val->as.string);
}

void dump_node(FILE* fp, ASTNode* ast) {
	//printf("Node: %p\n\t%p %p %p\n\n", ast, ast->parent, ast->left, ast->right);
	if (!ast) fprintf(fp, "ERROR\n");
	fprintf(fp, "\t\"%s:%d:%d\" ", ast->token->origin_file, ast->token->origin_line, ast->token->origin_char);
	fprintf(fp, "[label=\"'%s'\\n%s\\n%s:%d:%d\"",
				ast->token->content, getStr_token_t(ast->token->type),
				ast->token->origin_file, ast->token->origin_line, ast->token->origin_char
			);
	fprintf(fp, "shape=\"%s\"]\n", true ? "ellipse" : "rectangle");

	if (ast->left) {
		fprintf(fp, "\t\"%s:%d:%d\"->\"%s:%d:%d\" [label=\"L\"]\n",
					ast->token->origin_file, ast->token->origin_line, ast->token->origin_char,
					ast->left->token->origin_file, ast->left->token->origin_line, ast->left->token->origin_char
				);
		dump_node(fp, ast->left);
	}
	if (ast->right) {
		fprintf(fp, "\t\"%s:%d:%d\"->\"%s:%d:%d\" [label=\"R\"]\n",
					ast->token->origin_file, ast->token->origin_line, ast->token->origin_char,
					ast->right->token->origin_file, ast->right->token->origin_line, ast->right->token->origin_char
				);
		dump_node(fp, ast->right);
	}
}

void dump_tree(FILE* fp, ASTNode* rootNode) {
	fprintf(fp, "digraph G {\n");
	dump_node(fp, rootNode);
	fprintf(fp, "}\n");
}

void dump_variable_lookup(FILE* fp, VariableLookup* lookup) {
	fprintf(fp, "-- Symbol lookup --\n");
	for (size_t x = 0; x < lookup->_internal_size; x++) {
		fprintf(fp, "%-10s ", lookup->_names[x]);
		dump_value(fp, lookup->_values + x);
		fprintf(fp, "\n");
	}
}

void dump_interpreter(FILE* fp, Interpreter* terp) {
	for (size_t x = 0; x < terp->_top_index; x++) {
		fprintf(fp, "%3d ", x);
		dump_value(fp, terp->_stack + x);
		fprintf(fp, "\n");
	}
	dump_variable_lookup(fp, terp->lookup);
}

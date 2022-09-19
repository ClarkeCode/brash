#include <stdlib.h>
#include <stdbool.h>
#include <assert.h> //assert
#include <math.h> //fmod

#include "structs.h"
#include "enumerations.h"
#include "parser_ast.h"
#include "interpreter.h"
#include "variablelookup.h"

#include "dumpfunctions.h"

Interpreter* make_interpreter() {
	Interpreter* terp = (Interpreter*) calloc(1, sizeof(Interpreter));
	terp->lookup = make_variable_lookup();
	terp->_top_index = 0;
	terp->_max_index = 256;
	terp->_stack = (Value*) calloc(terp->_max_index, sizeof(Value));
	return terp;
}
void free_interpreter(Interpreter* terp) {
	if (!terp) return;
	if (terp->lookup) free_variable_lookup(terp->lookup);
	if (terp->_stack) free(terp->_stack);
	if (terp) free(terp);
}

Value pop(Interpreter* terp, FILE* fp) {
	if (terp->_top_index - 1 == (size_t)-1) {fprintf(stderr, "ERR: Stack Underflow\n");} //Stack underflow
	if (fp) {
		fprintf(fp, "POP:  ");
		dump_value(fp, terp->_stack + (terp->_top_index)-1);
		fprintf(fp, "\n");
	}
	return terp->_stack[--(terp->_top_index)];
}
void push(Interpreter* terp, Value val, FILE* fp) {
	if (terp->_top_index + 1 >= terp->_max_index) {fprintf(stderr, "ERR: Stack Overflow\n");} //Stack overflow
	if (fp) {
		fprintf(fp, "PUSH: ");
		dump_value(fp, &val);
		fprintf(fp, "\n");
	}
	terp->_stack[(terp->_top_index)++] = val; //Copy
}

void interpret(Interpreter* terp, ASTNode* node, FILE* fp) {
	if (!node) return;
	if (node->value.type != TYPE_NOT_VALUE) {
		push(terp, node->value, fp);
	}

	interpret(terp, node->left, fp);
	interpret(terp, node->right, fp);

	switch (node->token->type) {
		case OPERATOR_ADD: {
			Value rval = pop(terp, fp);
			Value lval = pop(terp, fp);
			Value nval = {TYPE_NUMBER, {.number=(lval.as.number + rval.as.number)}};
			if(fp) fprintf(fp, "ADD\n");
			push(terp, nval, fp);
		} break;

		case OPERATOR_SUB: {
			Value rval = pop(terp, fp);
			Value lval = pop(terp, fp);
			Value nval = {TYPE_NUMBER, {.number=(lval.as.number - rval.as.number)}};
			if(fp) fprintf(fp, "SUBTRACT\n");
			push(terp, nval, fp);
		} break;

		case OPERATOR_MULTIPLY: {
			Value rval = pop(terp, fp);
			Value lval = pop(terp, fp);
			Value nval = {TYPE_NUMBER, {.number=(lval.as.number * rval.as.number)}};
			if(fp) fprintf(fp, "MULTIPLY\n");
			push(terp, nval, fp);
		} break;

		case OPERATOR_DIVIDE: {
			Value rval = pop(terp, fp);
			Value lval = pop(terp, fp);
			Value nval = {TYPE_NUMBER, {.number=(lval.as.number / rval.as.number)}};
			if(fp) fprintf(fp, "DIVIDE\n");
			push(terp, nval, fp);
		} break;

		case OPERATOR_MODULO: {
			Value rval = pop(terp, fp);
			Value lval = pop(terp, fp);
			Value nval = {TYPE_NUMBER, {.number=(fmod(lval.as.number, rval.as.number))}};
			if(fp) fprintf(fp, "MODULO\n");
			push(terp, nval, fp);
		} break;

		case OPERATOR_EQUALITY: { //TODO: define equality for bool and string?
			Value rval = pop(terp, fp);
			Value lval = pop(terp, fp);
			Value nval = {TYPE_BOOLEAN, {.boolean=(lval.as.number == rval.as.number)}};
			if(fp) fprintf(fp, "EQUALITY\n");
			push(terp, nval, fp);
		} break;

		case OPERATOR_INEQUALITY: { //TODO: define equality for bool and string?
			Value rval = pop(terp, fp);
			Value lval = pop(terp, fp);
			Value nval = {TYPE_BOOLEAN, {.boolean=(lval.as.number != rval.as.number)}};
			if(fp) fprintf(fp, "INEQUALITY\n");
			push(terp, nval, fp);
		} break;

		case OPERATOR_LESSER: {
			Value rval = pop(terp, fp);
			Value lval = pop(terp, fp);
			Value nval = {TYPE_BOOLEAN, {.boolean=(lval.as.number < rval.as.number)}};
			if(fp) fprintf(fp, "LESSER\n");
			push(terp, nval, fp);
		} break;

		case OPERATOR_GREATER: {
			Value rval = pop(terp, fp);
			Value lval = pop(terp, fp);
			Value nval = {TYPE_BOOLEAN, {.boolean=(lval.as.number > rval.as.number)}};
			if(fp) fprintf(fp, "GREATER\n");
			push(terp, nval, fp);
		} break;

		case OPERATOR_LESSER_EQUAL: {
			Value rval = pop(terp, fp);
			Value lval = pop(terp, fp);
			Value nval = {TYPE_BOOLEAN, {.boolean=(lval.as.number <= rval.as.number)}};
			if(fp) fprintf(fp, "LESSER_EQUAL\n");
			push(terp, nval, fp);
		} break;

		case OPERATOR_GREATER_EQUAL: {
			Value rval = pop(terp, fp);
			Value lval = pop(terp, fp);
			Value nval = {TYPE_BOOLEAN, {.boolean=(lval.as.number >= rval.as.number)}};
			if(fp) fprintf(fp, "GREATER_EQUAL\n");
			push(terp, nval, fp);
		} break;

		case LOGICAL_AND: {
			Value rval = pop(terp, fp);
			Value lval = pop(terp, fp);
			Value nval = {TYPE_BOOLEAN, {.boolean=(lval.as.boolean && rval.as.boolean)}};
			if(fp) fprintf(fp, "LOGICAL_AND\n");
			push(terp, nval, fp);
		} break;

		case LOGICAL_OR: {
			Value rval = pop(terp, fp);
			Value lval = pop(terp, fp);
			Value nval = {TYPE_BOOLEAN, {.boolean=(lval.as.boolean || rval.as.boolean)}};
			if(fp) fprintf(fp, "LOGICAL_OR\n");
			push(terp, nval, fp);
		} break;

		case LOGICAL_XOR: {
			Value rval = pop(terp, fp);
			Value lval = pop(terp, fp);
			Value nval = {TYPE_BOOLEAN, {.boolean=(lval.as.boolean ^  rval.as.boolean)}};
			if(fp) fprintf(fp, "LOGICAL_XOR\n");
			push(terp, nval, fp);
		} break;

		case LOGICAL_NOT: {
			Value rval = pop(terp, fp);
			Value nval = {TYPE_BOOLEAN, {.boolean=(!rval.as.boolean)}};
			if(fp) fprintf(fp, "LOGICAL_NOT\n");
			push(terp, nval, fp);
		} break;

		case OPERATOR_ASSIGNMENT: {
			Value val = pop(terp, fp);
			//TODO: check node->left is an identifier
			lookup_add(terp->lookup, node->left->token->content, val);
			if (fp) {
				fprintf(fp, "ASSIGN_VARIABLE ");
				fprintf(fp, "'%s': - ", node->left->token->content);
				dump_value(fp, &val);
				fprintf(fp, "\n");
			}
		} break;

		case IDENTIFIER: {
			if (lookup_has(terp->lookup, node->token->content)) {
				push(terp, lookup_get(terp->lookup, node->token->content), fp);
			}
		} break;

		case STRING:
		case NUMBER:
		case BOOLEAN:
		case PAREN_OPEN:
		case PAREN_CLOSE:
		case BRACE_OPEN:
		case BRACE_CLOSE:
		case ALL_TOKENS:
			break;
	}
}

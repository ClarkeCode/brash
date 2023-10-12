#include <stack>
#include "compiler.hpp"

using namespace std;

// #include <cstdio>
// #include <stdlib.h>
// //The compiler version that I'm currently using doesn't support std::format, so I've rolled my own
// template<typename ...Args>
// string jankFormat(string const& format, Args... args) {
// 	size_t gogo = (size_t)snprintf(nullptr, 0, format.c_str(), args...) + 1;
// 	if (gogo <= 0) return "";
//
// 	char* buff = (char*) malloc(sizeof(char) * gogo);
// 	snprintf(buff, gogo, format.c_str(), args...);
// 	string output(buff);
// 	free(buff);
// 	return output;
// }

Compiler::Compiler(Lexer const &lexer) {
	lexStart = lexer.tokenSequence.cbegin();
	lexEnd = lexer.tokenSequence.cend();
}

constexpr const char* counterpart_string(token_t value) {
	switch (value) {
		case token_t::TK_PAREN_OPEN: return ")";
		case token_t::TK_BRACE_OPEN: return "}";
		case token_t::TK_SQUARE_BRACKET_OPEN: return "]";
		case token_t::TK_PAREN_CLOSE: return "(";
		case token_t::TK_BRACE_CLOSE: return "{";
		case token_t::TK_SQUARE_BRACKET_CLOSE: return "[";
		default: return "";
	}
	return "";
}
constexpr token_t counterpart(token_t value) {
	switch (value) {
		case token_t::TK_PAREN_OPEN: return token_t::TK_PAREN_CLOSE;
		case token_t::TK_BRACE_OPEN: return token_t::TK_BRACE_CLOSE;
		case token_t::TK_SQUARE_BRACKET_OPEN: return token_t::TK_SQUARE_BRACKET_CLOSE;
		case token_t::TK_PAREN_CLOSE: return token_t::TK_PAREN_OPEN;
		case token_t::TK_BRACE_CLOSE: return token_t::TK_BRACE_OPEN;
		case token_t::TK_SQUARE_BRACKET_CLOSE: return token_t::TK_SQUARE_BRACKET_OPEN;
		default: return token_t::TK_ERROR;
	}
	return token_t::TK_ERROR;
}
//Check that all ('s {'s and ['s have a matching ], }, or ) at some point
void performPairedTokenCheck(Lexer::TokenSequence::const_iterator start, Lexer::TokenSequence::const_iterator end) {
	stack<Token> st;

	for(Lexer::TokenSequence::const_iterator current = start; current < end; current++) {
		if (current->type == token_t::TK_PAREN_OPEN ||
			current->type == token_t::TK_BRACE_OPEN ||
			current->type == token_t::TK_SQUARE_BRACKET_OPEN) {
			st.push(*current);
			continue;
		}

		if (current->type == token_t::TK_PAREN_CLOSE ||
			current->type == token_t::TK_BRACE_CLOSE ||
			current->type == token_t::TK_SQUARE_BRACKET_CLOSE) {
			if (st.size() == 0) {
				cerr << "Parser Error: found an unpaired '" << current->content << "' at " << current->location << endl;
				continue;
			} 
			else if (st.top().type != counterpart(current->type)) {
				cerr << "Parser Error: " << current->location << endl << "\tBracketing mismatch - expected '" << counterpart_string(st.top().type) << "', found '" << current->content << "'."  << " Bracketing started with: '" << st.top().content << "' at " << st.top().location << endl;
				continue;
			}
			st.pop();
			continue;
		}
	}
	
	while (st.size() > 0) {
		cerr << "Parser Error: " << st.top().location << endl << "\tUnpaired '" << st.top().content << "', could not find a closing '" << counterpart_string(st.top().type) << "'." << endl;
		st.pop();
	}
}

void Compiler::process() {
	performPairedTokenCheck(lexStart, lexEnd);
};


void disassemble(std::ostream& os, ByteSequence const& bs) {
	for (ByteSequence::const_iterator current = bs.cbegin(); current < bs.end(); current++) {
		size_t offset = current - bs.cbegin();
		os << offset;

		OpCode currentOp = static_cast<OpCode>(*current);
		switch (currentOp) {
			case OP_TRUE:
			case OP_FALSE:
			case OP_NEGATE:
			case OP_ADD:
			case OP_SUBTRACT:
			case OP_MULTIPLY:
			case OP_DIVIDE:
			case OP_MODULO:
			case OP_EQUALS:
			case OP_GREATER:
			case OP_LESSER:
			case OP_NOT:
			case OP_AND:
			case OP_OR:
			case OP_XOR:
				os << enumStringLookup(currentOp) << endl;
				continue;
			
			default:
				os << enumStringLookup(currentOp) << "NOT IMPLEMENTED" << endl;
		}
	}
}

void Compiler::dump(std::ostream& os) {
	disassemble(os, bytecode);
};

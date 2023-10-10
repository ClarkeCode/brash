#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.hpp"

using namespace std;

ostream &operator<<(ostream &os, Location const &loc) {
	return os << loc.file << ":" << loc.line << ":" << loc.offset;
}

ostream &operator<<(ostream &os, Token const &tok) {
	return os << "Token(" << enumStringLookup(tok.type) << " '" << tok.content << "' @ " << tok.location << ")";
}

Lexer::Lexer(std::string filename) {
	ifstream fstream(filename);
	stringstream ss;
	ss << fstream.rdbuf();
	fileContents = ss.str();

	cursor.currentLoc = Location(filename);
	cursor.currentChar = fileContents.cbegin();
	cursor.endIt = fileContents.cend();
}

void Lexer::process() {
	Token tk(token_t::TK_EOF, "", cursor);
	do {
		tk = produceNextToken();
		tokenSequence.push_back(tk);
	} while (tk.type != token_t::TK_EOF);

}


const string WHITESPACE_CHARS = " \t";
const string VALID_NUMERIC_CHARS = "0123456789.";
const string VALID_IDENTIFIER_CHAR = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_";

bool doesStringContain(string const& haystack, const char needle) {
	return haystack.find(needle) != string::npos;
}

void Lexer::Cursor::skipAllWhitespace() {
	while (!atEnd() && doesStringContain(WHITESPACE_CHARS, *currentChar)) {
		advance(1);
	}
}

//If the next n characters match the provided literal, return true, otherwise return false
bool Lexer::Cursor::matchLiteral(std::string const& literal) {
	string::const_iterator current = currentChar;
	string::const_iterator lit = literal.cbegin();

	do {
		if ((current == endIt) || (*lit++ != *current++)) return false;
	} while (lit != literal.cend());

	return true;
}


#define RET_IF_MATCH(literal, tok_t) \
	if (cursor.matchLiteral(literal)) {\
		Token tk(tok_t, literal, cursor.currentLoc);\
		cursor.advanceLengthOfLiteral(literal);\
		return tk;\
	}

Token Lexer::produceNextToken() {
	cursor.skipAllWhitespace();

	//At end?
	if (cursor.atEnd()) {
		return Token(token_t::TK_EOF, "", cursor);
	}

	//Is newline?
	if (*cursor.currentChar == '\n') {
		Token tk(token_t::TK_NEWLINE, "", cursor.currentLoc);
		cursor.advanceOneStartNewline();
		return tk;
	}

	//Ignore comments

	//Is numeric literal?

	//Is string literal?

	//Keywords, operators, and other builtins
	RET_IF_MATCH(";",     token_t::TK_SEMICOLON);
	RET_IF_MATCH(",",     token_t::TK_COMMA);

	RET_IF_MATCH("true",  token_t::TK_BOOLEAN);
	RET_IF_MATCH("false", token_t::TK_BOOLEAN);

	RET_IF_MATCH("(", token_t::TK_PAREN_OPEN);
	RET_IF_MATCH(")", token_t::TK_PAREN_CLOSE);
	RET_IF_MATCH("[", token_t::TK_SQUARE_BRACKET_OPEN);
	RET_IF_MATCH("]", token_t::TK_SQUARE_BRACKET_CLOSE);
	RET_IF_MATCH("{", token_t::TK_BRACE_OPEN);
	RET_IF_MATCH("}", token_t::TK_BRACE_CLOSE);

	RET_IF_MATCH("+", token_t::TK_ADD);
	RET_IF_MATCH("-", token_t::TK_SUB);
	RET_IF_MATCH("*", token_t::TK_MULTIPLY);
	RET_IF_MATCH("/", token_t::TK_DIVIDE);
	RET_IF_MATCH("%", token_t::TK_MODULO);
	RET_IF_MATCH("&&",    token_t::TK_AND);
	RET_IF_MATCH("||",    token_t::TK_OR);
	RET_IF_MATCH("^^",    token_t::TK_XOR);

	RET_IF_MATCH("if",      token_t::TK_IF);
	RET_IF_MATCH("else",    token_t::TK_ELSE);
	RET_IF_MATCH("for",     token_t::TK_FOR);
	RET_IF_MATCH("while",   token_t::TK_WHILE);

	RET_IF_MATCH("def",    token_t::TK_DEC_FUNCTION);
	RET_IF_MATCH("func",   token_t::TK_DEC_FUNCTION);
	RET_IF_MATCH("return", token_t::TK_RETURN);
	RET_IF_MATCH("ret",    token_t::TK_RETURN);

	//Operators or keywords with potential overlaps
	RET_IF_MATCH("==", token_t::TK_EQUALITY);
	RET_IF_MATCH("!=", token_t::TK_INEQUALITY);
	RET_IF_MATCH("!",  token_t::TK_NOT);
	RET_IF_MATCH("=",  token_t::TK_ASSIGNMENT);
	RET_IF_MATCH("<=", token_t::TK_LESSER_EQUAL);
	RET_IF_MATCH(">=", token_t::TK_GREATER_EQUAL);
	RET_IF_MATCH("<",  token_t::TK_LESSER);
	RET_IF_MATCH(">",  token_t::TK_GREATER);
	RET_IF_MATCH("▷", token_t::TK_FORWARD_VALUE_RIGHT); //NOTE: ▷ (U+25B7 WHITE RIGHT-POINTING TRIANGLE)
	RET_IF_MATCH("|>", token_t::TK_FORWARD_VALUE_RIGHT); //NOTE: If possible with font ligatures, display as ▷ (U+25B7)
	RET_IF_MATCH("|",  token_t::TK_FORWARD_VALUE_LEFT);

	//  //Is identifier?
	// 	if (char_in(currentChar(), VALID_IDENTIFIER_CHAR)) {
	// 	while (currentChar() != '\0' && char_in(currentChar(), VALID_IDENTIFIER_CHAR)) {
	// 		lexer.current++;
	// 	}
	// 	lexer.current++;
	// 	nSetView(content, lexer.start, lexer.current-lexer.start-1);
	// 	lexer.location.offset += (lexer.current-lexer.start);
	// 	return TK_IDENTIFIER;
	// }

	stringstream ss;
	ss << "Unexpected character '" << *cursor.currentChar << "'";	
	Token tk(token_t::TK_ERROR, ss.str(), cursor);
	cursor.advance(1);
	return tk;
}

#undef RET_IF_MATCH

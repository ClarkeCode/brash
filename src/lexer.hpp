#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "basictypes.hpp"

//♪♫ Won't - you - take - me - to ♪♫ MACRO TOWN!
//See https://www.drdobbs.com/the-new-c-x-macros/184401387 for a detailed explaination of 'X macros'
//Objective is to unite the definition of enum values with the definition of their string lookups
#define TOKEN_TABLE             \
	X(TK_ERROR),                \
	X(TK_EOF),                  \
	X(TK_NEWLINE),              \
	X(TK_SEMICOLON),            \
	X(TK_COMMA),                \
	\
	/*Primitives*/ \
	X(TK_NUMBER),               \
	X(TK_STRING),               \
	X(TK_BOOLEAN),              \
	X(TK_IDENTIFIER),           \
	\
	/*Scope and bracketing*/    \
	X(TK_PAREN_OPEN),           \
	X(TK_PAREN_CLOSE),          \
	X(TK_SQUARE_BRACKET_OPEN),  \
	X(TK_SQUARE_BRACKET_CLOSE), \
	X(TK_BRACE_OPEN),           \
	X(TK_BRACE_CLOSE),          \
	\
	/*Arithmetic operators*/    \
	X(TK_ADD),                  \
	X(TK_SUB),                  \
	X(TK_MULTIPLY),             \
	X(TK_DIVIDE),               \
	X(TK_MODULO),               \
	X(TK_ASSIGNMENT),           \
	\
	/*Logical operators*/       \
	X(TK_NOT),                  \
	X(TK_AND),                  \
	X(TK_OR),                   \
	X(TK_XOR),                  \
	\
	/*Comparison operators*/    \
	X(TK_EQUALITY),             \
	X(TK_INEQUALITY),           \
	X(TK_LESSER),               \
	X(TK_GREATER),              \
	X(TK_LESSER_EQUAL),         \
	X(TK_GREATER_EQUAL),        \
	\
	/*Control flow and loops*/  \
	X(TK_IF),                   \
	X(TK_ELSE),                 \
	X(TK_FOR),                  \
	X(TK_WHILE),                \
	\
	/*Function-related items*/  \
	X(TK_DEC_FUNCTION),         \
	X(TK_RETURN),               \
	X(TK_FORWARD_VALUE_LEFT),   \
	X(TK_FORWARD_VALUE_RIGHT),  \

#define X(text) text
enum class token_t : byte_t {
	TOKEN_TABLE
	// TK_VAR,
	// TK_PRINT,
	// TK_TYPE_NUMBER,
	// TK_TYPE_BOOLEAN,
	// TK_TYPE_STRING,
	// TK_TYPE_FUNCTION,
	// TK_TYPE_CUSTOM,
};
#undef X
namespace detail {
	#define X(text) #text
	constexpr const char* token_t_string_lookup[] = {
		TOKEN_TABLE
	};
	#undef X
};
#undef TOKEN_TABLE

constexpr const char* enumStringLookup(token_t value) { return detail::token_t_string_lookup[(std::size_t)value]; }

struct Location {
	std::string file; //May be empty when input is not from file; ie: REPL
	size_t line;
	size_t offset;

	Location(std::string fileName = "", size_t line = 0, size_t offset = 0) : file(fileName), line(line), offset(offset) {}

	friend std::ostream& operator<<(std::ostream& os, Location const& loc);
};

struct Token {
	token_t type;
	std::string content;
	Location location;

	Token(token_t tkType, std::string tkContent, Location tkLocation) : type(tkType), content(tkContent), location(tkLocation) {}

	friend std::ostream& operator<<(std::ostream& os, Token const& tok);
};

struct Lexer {
	private:
	struct Cursor {
		Location currentLoc;
		std::string::const_iterator currentChar;
		std::string::const_iterator endIt;

		operator Location() const { return currentLoc; }

		inline void advance(std::size_t value) { currentLoc.offset += value; currentChar += value; }
		inline void advanceLengthOfLiteral(std::string literal) { advance(literal.length()); }
		inline void advanceOneStartNewline() { currentLoc.offset = 0; currentLoc.line++; currentChar += 1; }
		inline bool atEnd() const { return currentChar == endIt; }
		void skipAllWhitespace();
		bool matchLiteral(std::string const& literal);
	} cursor;

	std::string fileContents;
	Token produceNextToken();

	public:
	std::string filename;
	std::vector<Token> tokenSequence;

	Lexer(std::string filename);
	void process();
};
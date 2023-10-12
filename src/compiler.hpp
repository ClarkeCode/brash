#pragma once
#include <vector>
#include <iostream>
#include "basictypes.hpp"
#include "lexer.hpp"

#define OPCODE_TABLE     \
	X(OP_NUMBER),        \
	X(OP_STRING),        \
	\
	X(OP_NEGATE),        \
	X(OP_ADD),           \
	X(OP_SUBTRACT),      \
	X(OP_MULTIPLY),      \
	X(OP_DIVIDE),        \
	X(OP_MODULO),        \
	\
	X(OP_TRUE),          \
	X(OP_FALSE),         \
	\
	X(OP_EQUALS),        \
	X(OP_GREATER),       \
	X(OP_LESSER),        \
	\
	X(OP_NOT),           \
	X(OP_AND),           \
	X(OP_OR),            \
	X(OP_XOR),           \
	\
	X(OP_DEF_VARIABLE),  \
	X(OP_SET_VARIABLE),  \
	X(OP_GET_VARIABLE),  \
	X(OP_POP),           \
	\
	X(OP_ENTER_SCOPE),   \
	X(OP_EXIT_SCOPE),    \
	\
	X(OP_PRINT),         \
	X(OP_SMALL_JUMP),          \
	X(OP_JUMP),          \
	X(OP_BIG_JUMP),          \
	X(OP_PERFORM_JUMP_IF_FALSE), \
	X(OP_LOOP),          \
	\
	X(OP_DEC_FUNCTION),  \
	X(OP_FUNCTION_CALL), \
	X(OP_RETURN),        \

#define X(text) text
enum OpCode : byte_t {
	OPCODE_TABLE
};
#undef X

namespace detail {
	#define X(text) #text
	constexpr const char* OpCode_string_lookup[] = {
		OPCODE_TABLE
	};
	#undef X
}
#undef OPCODE_TABLE

constexpr const char* enumStringLookup(OpCode value) { return detail::OpCode_string_lookup[(std::size_t)value]; }

struct Compiler {
	Lexer::TokenSequence::const_iterator lexStart, lexEnd; //Priv
	ByteSequence bytecode;

	Compiler(Lexer const& lexer);
	void process();
	void dump(std::ostream& os);
};
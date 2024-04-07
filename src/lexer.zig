pub const TokenType = enum {
	Error,
	EOF,
	Newline,
	Semicolon,
	Comma,

	//Primitives
	Number,
	String,
	Boolean,
	Identifier,

	//Scope and Bracketing
	ParenOpen,
	ParenClose,
	SquareBracketOpen,
	SquareBracketClose,
	BraceOpen,
	BraceClose,

	//Arithmetic Operators
	Add,
	Subtract,
	Multiply,
	Divide,
	Modulo,
	Assignment,

	//Logical Operators
	Not,
	And,
	Or,
	Xor,

	//Comparison Operators
	Equality,
	Inequality,
	Lesser,
	Greater,
	LesserEqual,
	GreaterEqual,

	//Control flow and loops
	If,
	Else,
	For,
	While,

	pub fn toString(self: TokenType) []const u8 { return @tagName(self); }
};

const std = @import("std");

pub const Token = struct {
	kind: TokenType,
	value: []const u8,

	//TODO: This probably leaks memory, but is good enough for now
	pub fn toString(self: Token) []const u8 {
		return std.fmt.allocPrint(std.heap.page_allocator, "Token <{s}> '{s}'" ++ "aaa", .{self.kind.toString(), self.value}) catch "";
	}

	pub fn isType(self: Token, tkTypes: []const TokenType) bool {
		for (tkTypes) |elem| {
			if (self.kind == elem) return true;
		}
		return false;
	}
};


const Lexer = struct {
	tokens: []Token,
	source: []u8,
	position: u64,
};

pub fn Tokenize(source: []const u8) []Token {
	var lex: Lexer = .{.source = source, .position = 0};

	return lex.tokens;
}




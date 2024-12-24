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

	Var,
	Const,

	Function,
	Return,

	pub fn toString(self: TokenType) []const u8 { return @tagName(self); }
};


pub const Token = struct {
	kind: TokenType,
	text: []const u8,

	//TODO: This probably leaks memory, and a page allocator is overkill, but works well enough for now
	pub fn toString(self: Token) []const u8 {
		return std.fmt.allocPrint(std.heap.page_allocator, "Token <{s}> '{s}'", .{self.kind.toString(), self.text}) catch "";
	}

	pub fn isType(self: Token, testType: TokenType) bool { return self.kind == testType; }
	pub fn isTypes(self: Token, typeTuple: anytype) bool {
		inline for (typeTuple) |value| {
			if (self.kind == value) { return true; }
		}
		return false;
	}
};
pub fn makeToken(kind: TokenType, text: []const u8) Token {
	return Token{.kind = kind, .text = text};
}

const std = @import("std");
pub const TokenList_t = std.ArrayList(Token);


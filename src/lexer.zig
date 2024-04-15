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
		return std.fmt.allocPrint(std.heap.page_allocator, "Token <{s}> '{s}'", .{self.kind.toString(), self.value}) catch "";
	}

	pub fn isType(self: Token, kind1: TokenType) bool { return self.kind == kind1; }
	pub fn isType2(self: Token, kind1: TokenType, kind2: TokenType) bool { return self.kind == kind1 or self.kind == kind2; }
	pub fn isType3(self: Token, kind1: TokenType, kind2: TokenType, kind3: TokenType) bool { return self.kind == kind1 or self.kind == kind2 or self.kind == kind3; }
	pub fn isType4(self: Token, kind1: TokenType, kind2: TokenType, kind3: TokenType, kind4: TokenType) bool { return self.isType3(kind1, kind2, kind3) or self.kind == kind4; }
	pub fn isTypes(self: Token, tkTypes: []const TokenType) bool {
		for (tkTypes) |elem| {
			if (self.kind == elem) return true;
		}
		return false;
	}
};
pub fn makeToken(kind: TokenType, value: []const u8) Token {
	return Token{.kind = kind, .value = value};
}



fn streq(a: []const u8, b: []const u8) bool {
	return std.mem.eql(u8, a, b);
}

const TokenList_t = std.ArrayList(Token);
pub const Lexer = struct {
	tokens: TokenList_t,
	source: []const u8,
	position: usize = 0,

	fn advanceN(self: *Lexer, advance: u64) void { self.position += advance; }
	fn getRemainder(self: *Lexer) []const u8 { return self.source[self.position..]; }
	fn atEOF(self: *Lexer) bool { return self.position >= self.source.len; }

	pub fn deinit(self: *Lexer) void { self.tokens.deinit(); }
};

fn hasBasicMatch(needle: []const u8, haystack: []const u8) bool {
	return if (needle.len <= haystack.len) streq(needle, haystack[0..needle.len]) else false;
}

pub fn Tokenize(lexer: *Lexer) !void {
	while (!lexer.atEOF()) {
		const remainder: []const u8 = lexer.getRemainder();
		
		if (hasBasicMatch("==", remainder)) {
			try lexer.tokens.append(makeToken(.Equality, "=="));
			lexer.advanceN(2);
		}
		else if (hasBasicMatch("+", remainder)) {
			try lexer.tokens.append(makeToken(.Add, "+"));
			lexer.advanceN(1);
		}
		else if (hasBasicMatch("1", remainder) or hasBasicMatch("2", remainder) or hasBasicMatch("3", remainder)) {
			try lexer.tokens.append(makeToken(.Number, remainder[0..1]));
			lexer.advanceN(1);
		}
	}
}

pub fn InitLexer(allocator: std.mem.Allocator, source: []const u8) Lexer {
	var lex: Lexer = .{.tokens = TokenList_t.init(allocator), .source = source, .position = 0};
	return lex;
}




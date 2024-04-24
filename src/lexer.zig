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

const std = @import("std");
const strutil = @import("stringutils.zig");

pub const Token = struct {
	kind: TokenType,
	value: []const u8,

	//TODO: This probably leaks memory, but is good enough for now
	pub fn toString(self: Token) []const u8 {
		return std.fmt.allocPrint(std.heap.page_allocator, "Token <{s}> '{s}'", .{self.kind.toString(), self.value}) catch "";
	}

	pub fn isTypes(self: Token, typeTuple: anytype) bool {
		inline for (typeTuple) |value| {
			if (self.kind == value) { return true; }
		}
		return false;
	}
};
pub fn makeToken(kind: TokenType, value: []const u8) Token {
	return Token{.kind = kind, .value = value};
}





const LexerError = error {
	UnterminatedString,
};

const matcher = struct {
	fn matchLiteral(literal: []const u8, haystack: []const u8) ?[]const u8 {
		if (strutil.fuzzyEqual(literal, haystack)) return literal;
		return null;
	}

	fn matchWhitespace(haystack: []const u8) ?[]const u8 {
		var incrementor: usize = 0;
		for (haystack, 0..) |value, i| {
			if (strutil.findFirstInstance(" \t\n", value)) |_| {
				incrementor += 1;
			}
			else {
				if (i != 0) break;
				return null;
			}
		}
		return haystack[0..incrementor];
	}

	fn matchNumber(haystack: []const u8) ?[]const u8 {
		// var anyNum = false;
		var hasDecimal = false;
		var incrementor: usize = 0;
		for (haystack, 0..) |value, i| {
			if (strutil.findFirstInstance("0123456789.", value) != null) {
				if (value == '.' and !hasDecimal) { hasDecimal = true; }
				else if (value == '.' and hasDecimal) { break; } //If there is a second period, only use the first '123.321653.' -> '123.321653'
				incrementor += 1;
			}
			else {
				if (i == 0) { return null; } //If i is 0, the first character was not numeric and there is no match
				break; //Otherwise, stop searching and return the subslice
			}
		}
		if (incrementor == 1 and haystack[0] == '.') return null; //Ensure that a match of just '.' is not a valid number
		return haystack[0..incrementor];
	}

	fn matchIdentifier(haystack: []const u8) ?[]const u8 {
		var incrementor: usize = 0;
		if (haystack.len == 0) return null;
		if (strutil.findFirstInstance("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", haystack[0])) |_| {
			incrementor += 1;
		}
		else return null;

		for (haystack[1..]) |value| {
			if (strutil.findFirstInstance("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-0123456789", value)) |_| {
				incrementor += 1;
			}
			else break;
		}
		return haystack[0..incrementor];
	}

	fn matchString(haystack: []const u8) LexerError !?[]const u8 {
		var incrementor: usize = 0;
		if (haystack[0] != '"') {
			return null;
		}
		else incrementor += 1;

		for (haystack[1..]) |value| {
			//TODO: handle character escapes with a backslash '\'
			incrementor += 1;
			if (value == '"') break;
		}

		//If there is not a closing quote, error out
		if (haystack[incrementor-1] != '"') {
			return error.UnterminatedString;
		}
		return haystack[0..incrementor];
	}
};

test "matchNumber" {
	try std.testing.expect(matcher.matchNumber("abcd") == null);
	try std.testing.expectEqualSlices(u8, "1234", matcher.matchNumber("1234abcd").?);
	try std.testing.expectEqualSlices(u8, "1234.56", matcher.matchNumber("1234.56abcd").?);
	try std.testing.expectEqualSlices(u8, "1234.56", matcher.matchNumber("1234.56.abcd").?);
	try std.testing.expectEqualSlices(u8, "1234.", matcher.matchNumber("1234.abcd").?);
	try std.testing.expectEqualSlices(u8, ".1234", matcher.matchNumber(".1234abcd").?);
	try std.testing.expect(matcher.matchNumber(".abcd") == null);
}

test "matchIdentifier" {
	try std.testing.expect(matcher.matchIdentifier("-john") == null);
	try std.testing.expect(matcher.matchIdentifier("0john") == null);
	try std.testing.expect(matcher.matchIdentifier("9john") == null);
	try std.testing.expectEqualSlices(u8, "john", matcher.matchIdentifier("john").?);
	try std.testing.expectEqualSlices(u8, "John", matcher.matchIdentifier("John").?);
	try std.testing.expectEqualSlices(u8, "jOhn", matcher.matchIdentifier("jOhn").?);
	try std.testing.expectEqualSlices(u8, "jOhn_James33-Jeff", matcher.matchIdentifier("jOhn_James33-Jeff").?);
}

test "matchString" {
	try std.testing.expectError(error.UnterminatedString, matcher.matchString("\"unterminatedstring"));
	try std.testing.expect(try matcher.matchString("unterminatedstring\"") == null);

	const successfulString = try matcher.matchString("\"properly terminated string\"");
	try std.testing.expectEqualSlices(u8, "\"properly terminated string\"", successfulString.?);
}





const TokenList_t = std.ArrayList(Token);
pub const Lexer = struct {
	tokens: TokenList_t,
	source: []const u8,
	position: usize = 0,

	fn advanceN(self: *Lexer, advance: usize) void { self.position += advance; }
	fn getRemainder(self: *Lexer) []const u8 { return self.source[self.position..]; }
	fn atEOF(self: *Lexer) bool { return self.position >= self.source.len; }
	fn pushNewToken(self: *Lexer, kind: TokenType, value: []const u8) !void { try self.tokens.append(makeToken(kind, value)); }

	pub fn init(allocator: std.mem.Allocator, source: []const u8) Lexer {
		return .{.tokens = TokenList_t.init(allocator), .source = source, .position = 0};
	}
	pub fn deinit(self: *Lexer) void { self.tokens.deinit(); }

	pub fn tokenize(lexer: *Lexer) !void {
		while (!lexer.atEOF()) {
			const remainder: []const u8 = lexer.getRemainder();
			
			if (matcher.matchWhitespace(remainder)) |match| {
				lexer.advanceN(match.len);
				continue;
			}

			if (matcher.matchLiteral("==", remainder)) |match| {
				try lexer.pushNewToken(.Equality, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("(", remainder)) |match| {
				try lexer.pushNewToken(.ParenOpen, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral(")", remainder)) |match| {
				try lexer.pushNewToken(.ParenClose, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("[", remainder)) |match| {
				try lexer.pushNewToken(.SquareBracketOpen, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("]", remainder)) |match| {
				try lexer.pushNewToken(.SquareBracketClose, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("{", remainder)) |match| {
				try lexer.pushNewToken(.BraceOpen, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("}", remainder)) |match| {
				try lexer.pushNewToken(.BraceClose, match);
				lexer.advanceN(match.len);
			}

			//Aritmetic
			else if (matcher.matchLiteral("+", remainder)) |match| {
				try lexer.pushNewToken(.Add, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("-", remainder)) |match| {
				try lexer.pushNewToken(.Subtract, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("*", remainder)) |match| {
				try lexer.pushNewToken(.Multiply, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("/", remainder)) |match| {
				try lexer.pushNewToken(.Divide, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("%", remainder)) |match| {
				try lexer.pushNewToken(.Modulo, match);
				lexer.advanceN(match.len);
			}

			else if (matcher.matchLiteral(">=", remainder)) |match| {
				try lexer.pushNewToken(.GreaterEqual, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral(">", remainder)) |match| {
				try lexer.pushNewToken(.Greater, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("<=", remainder)) |match| {
				try lexer.pushNewToken(.LesserEqual, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("<", remainder)) |match| {
				try lexer.pushNewToken(.Lesser, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("==", remainder)) |match| {
				try lexer.pushNewToken(.Equality, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("=", remainder)) |match| {
				try lexer.pushNewToken(.Assignment, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("!=", remainder)) |match| {
				try lexer.pushNewToken(.Inequality, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("!", remainder)) |match| {
				try lexer.pushNewToken(.Not, match);
				lexer.advanceN(match.len);
			}

			else if (matcher.matchLiteral("&&", remainder)) |match| {
				try lexer.pushNewToken(.And, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("||", remainder)) |match| {
				try lexer.pushNewToken(.Or, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("^^", remainder)) |match| {
				try lexer.pushNewToken(.Xor, match);
				lexer.advanceN(match.len);
			}



			//Keywords
			else if (matcher.matchLiteral("if", remainder)) |match| {
				try lexer.pushNewToken(.If, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("else", remainder)) |match| {
				try lexer.pushNewToken(.Else, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("for", remainder)) |match| {
				try lexer.pushNewToken(.For, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("while", remainder)) |match| {
				try lexer.pushNewToken(.While, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("var", remainder)) |match| {
				try lexer.pushNewToken(.Var, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("const", remainder)) |match| {
				try lexer.pushNewToken(.Const, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("true", remainder)) |match| {
				try lexer.pushNewToken(.Boolean, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("false", remainder)) |match| {
				try lexer.pushNewToken(.Boolean, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("func", remainder)) |match| {
				try lexer.pushNewToken(.Function, match);
				lexer.advanceN(match.len);
			}
			else if (matcher.matchLiteral("return", remainder)) |match| {
				try lexer.pushNewToken(.Return, match);
				lexer.advanceN(match.len);
			}

			//Primitives
			else if (matcher.matchNumber(remainder)) |num| {
				try lexer.pushNewToken(.Number, num);
				lexer.advanceN(num.len);
			}
			else if (matcher.matchIdentifier(remainder)) |identifier| {
				try lexer.pushNewToken(.Identifier, identifier);
				lexer.advanceN(identifier.len);
			}
			else if (try matcher.matchString(remainder)) |string| {
				try lexer.pushNewToken(.String, string[1..(string.len-1)]);
				lexer.advanceN(string.len);
			}

			else {
				unreachable;
			}
		}
	}

};



test "tokenize" {
	const alloc = std.testing.allocator;

	{
		var lex = Lexer.init(alloc, "1+2+3+4");
		try lex.tokenize();
		defer lex.deinit();

		const expected = .{
			makeToken(.Number, "1"),
			makeToken(.Add, "+"),
			makeToken(.Number, "2"),
			makeToken(.Add, "+"),
			makeToken(.Number, "3"),
			makeToken(.Add, "+"),
			makeToken(.Number, "4"),
		};

		inline for (expected, 0..) |value, i| {
			try std.testing.expectEqualDeep(value, lex.tokens.items[i]);
		}
	}

	{
		var lex = Lexer.init(alloc, "123.45 200 = ()[]{}+-*/%!&&||^^== != <<= >>= if\t else\n var const func return");
		try lex.tokenize();
		defer lex.deinit();

		const expected = .{
			makeToken(.Number, "123.45"),
			makeToken(.Number, "200"),
			makeToken(.Assignment, "="),
			makeToken(.ParenOpen, "("),
			makeToken(.ParenClose, ")"),
			makeToken(.SquareBracketOpen, "["),
			makeToken(.SquareBracketClose, "]"),
			makeToken(.BraceOpen, "{"),
			makeToken(.BraceClose, "}"),
			makeToken(.Add, "+"),
			makeToken(.Subtract, "-"),
			makeToken(.Multiply, "*"),
			makeToken(.Divide, "/"),
			makeToken(.Modulo, "%"),
			makeToken(.Not, "!"),
			makeToken(.And, "&&"),
			makeToken(.Or, "||"),
			makeToken(.Xor, "^^"),
			makeToken(.Equality, "=="),
			makeToken(.Inequality, "!="),
			makeToken(.Lesser, "<"),
			makeToken(.LesserEqual, "<="),
			makeToken(.Greater, ">"),
			makeToken(.GreaterEqual, ">="),
			makeToken(.If, "if"),
			makeToken(.Else, "else"),
			makeToken(.Var, "var"),
			makeToken(.Const, "const"),
			makeToken(.Function, "func"),
			makeToken(.Return, "return"),
		};

		inline for (expected, 0..) |value, i| {
			try std.testing.expectEqualDeep(value, lex.tokens.items[i]);
		}
	}
	
	{
		var lex = Lexer.init(alloc, "const myCoolVariable2 = \"rock on!\"");
		try lex.tokenize();
		defer lex.deinit();

		const expected = .{
			makeToken(.Const, "const"),
			makeToken(.Identifier, "myCoolVariable2"),
			makeToken(.Assignment, "="),
			makeToken(.String, "rock on!"),
		};

		inline for (expected, 0..) |value, i| {
			try std.testing.expectEqualDeep(value, lex.tokens.items[i]);
		}
	}
}




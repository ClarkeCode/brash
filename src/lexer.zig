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


const TokenList_t = std.ArrayList(Token);
pub const Lexer = struct {
	tokens: TokenList_t,
	source: []const u8,
	position: usize = 0,

	fn advanceN(self: *Lexer, advance: u64) void { self.position += advance; }
	fn getRemainder(self: *Lexer) []const u8 { return self.source[self.position..]; }
	fn atEOF(self: *Lexer) bool { return self.position >= self.source.len; }
	fn pushNewToken(self: *Lexer, kind: TokenType, value: []const u8) !void { try self.tokens.append(makeToken(kind, value)); }

	pub fn init(allocator: std.mem.Allocator, source: []const u8) Lexer {
		return .{.tokens = TokenList_t.init(allocator), .source = source, .position = 0};
	}
	pub fn deinit(self: *Lexer) void { self.tokens.deinit(); }
};

fn hasBasicMatch(needle: []const u8, haystack: []const u8) ?[]const u8 {
	if (strutil.fuzzyEqual(needle, haystack)) return needle;
	return null;
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

test "matchNumber" {
	try std.testing.expect(matchNumber("abcd") == null);
	try std.testing.expectEqualSlices(u8, "1234", matchNumber("1234abcd").?);
	try std.testing.expectEqualSlices(u8, "1234.56", matchNumber("1234.56abcd").?);
	try std.testing.expectEqualSlices(u8, "1234.56", matchNumber("1234.56.abcd").?);
	try std.testing.expectEqualSlices(u8, "1234.", matchNumber("1234.abcd").?);
	try std.testing.expectEqualSlices(u8, ".1234", matchNumber(".1234abcd").?);
	try std.testing.expect(matchNumber(".abcd") == null);
}

pub fn Tokenize(lexer: *Lexer) !void {
	while (!lexer.atEOF()) {
		const remainder: []const u8 = lexer.getRemainder();
		
		if (matchWhitespace(remainder)) |match| {
			lexer.advanceN(match.len);
			continue;
		}

		if (hasBasicMatch("==", remainder)) |match| {
			try lexer.pushNewToken(.Equality, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("(", remainder)) |match| {
			try lexer.pushNewToken(.ParenOpen, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch(")", remainder)) |match| {
			try lexer.pushNewToken(.ParenClose, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("[", remainder)) |match| {
			try lexer.pushNewToken(.SquareBracketOpen, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("]", remainder)) |match| {
			try lexer.pushNewToken(.SquareBracketClose, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("{", remainder)) |match| {
			try lexer.pushNewToken(.BraceOpen, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("}", remainder)) |match| {
			try lexer.pushNewToken(.BraceClose, match);
			lexer.advanceN(match.len);
		}

		//Aritmetic
		else if (hasBasicMatch("+", remainder)) |match| {
			try lexer.pushNewToken(.Add, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("-", remainder)) |match| {
			try lexer.pushNewToken(.Subtract, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("*", remainder)) |match| {
			try lexer.pushNewToken(.Multiply, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("/", remainder)) |match| {
			try lexer.pushNewToken(.Divide, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("%", remainder)) |match| {
			try lexer.pushNewToken(.Modulo, match);
			lexer.advanceN(match.len);
		}

		else if (hasBasicMatch(">=", remainder)) |match| {
			try lexer.pushNewToken(.GreaterEqual, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch(">", remainder)) |match| {
			try lexer.pushNewToken(.Greater, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("<=", remainder)) |match| {
			try lexer.pushNewToken(.LesserEqual, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("<", remainder)) |match| {
			try lexer.pushNewToken(.Lesser, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("==", remainder)) |match| {
			try lexer.pushNewToken(.Equality, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("=", remainder)) |match| {
			try lexer.pushNewToken(.Assignment, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("!=", remainder)) |match| {
			try lexer.pushNewToken(.Inequality, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("!", remainder)) |match| {
			try lexer.pushNewToken(.Not, match);
			lexer.advanceN(match.len);
		}

		else if (hasBasicMatch("&&", remainder)) |match| {
			try lexer.pushNewToken(.And, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("||", remainder)) |match| {
			try lexer.pushNewToken(.Or, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("^^", remainder)) |match| {
			try lexer.pushNewToken(.Xor, match);
			lexer.advanceN(match.len);
		}



		//Keywords
		else if (hasBasicMatch("if", remainder)) |match| {
			try lexer.pushNewToken(.If, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("else", remainder)) |match| {
			try lexer.pushNewToken(.Else, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("for", remainder)) |match| {
			try lexer.pushNewToken(.For, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("while", remainder)) |match| {
			try lexer.pushNewToken(.While, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("var", remainder)) |match| {
			try lexer.pushNewToken(.Var, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("const", remainder)) |match| {
			try lexer.pushNewToken(.Const, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("true", remainder)) |match| {
			try lexer.pushNewToken(.Boolean, match);
			lexer.advanceN(match.len);
		}
		else if (hasBasicMatch("false", remainder)) |match| {
			try lexer.pushNewToken(.Boolean, match);
			lexer.advanceN(match.len);
		}

		//Primitives
		else if (matchNumber(remainder)) |num| {
			try lexer.pushNewToken(.Number, num);
			lexer.advanceN(num.len);
		}
	}
}

test "tokenize" {
	const alloc = std.testing.allocator;

	{
		var lex = Lexer.init(alloc, "1+2+3+4");
		defer lex.deinit();
		try Tokenize(&lex);

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
		var lex = Lexer.init(alloc, "123.45 200 = ()[]{}+-*/%!&&||^^== != <<= >>= if\t else\n var const");
		defer lex.deinit();
		try Tokenize(&lex);

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
		};

		inline for (expected, 0..) |value, i| {
			try std.testing.expectEqualDeep(value, lex.tokens.items[i]);
		}
	}
}




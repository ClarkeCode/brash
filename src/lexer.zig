const std = @import("std");

pub const TokenType = enum {
	//Reserved
	Var,   //declaration
	Const, //declaration
	Assignment,
	EndStatement,
	Print,
	//Primitives
	Number,
	Identifier,
	//Arithmetic Operators
	Addition,

	pub fn toSlice(self: TokenType) []const u8 { return @tagName(self); }
};

pub const Token = struct {
	kind: TokenType,
	text: []const u8,

	pub fn toSlice(self: Token) []const u8 {
		//TODO: Ugly and overkill, but good enough for now
		return std.fmt.allocPrint(std.heap.page_allocator, "Token <{s}> '{s}'", .{self.kind.toSlice(), self.text}) catch "";
	}
	pub fn isType(self: Token, testType: TokenType) bool { return self.kind == testType; }
	pub fn isTypes(self: Token, typeTuple: anytype) bool {
		inline for (typeTuple) |value| {
			if (self.kind == value) { return true; }
		}
		return false;
	}
};

///It is the responsibility of the caller to deallocate returned Token slice
pub fn lex(alloc: std.mem.Allocator, textinput: []const u8) ![]Token {
	var tokenList = std.ArrayList(Token).init(alloc);

	var it = std.mem.tokenizeAny(u8, textinput, " \n");
	//std.debug.print("{*}\n", .{textinput.ptr});
	while (it.next()) |text| {
		////TODO: For parsing errors To track the textual origin of a Token, it can be computed from the byte offset
		////const byteOffsetOfText = @intFromPtr(text.ptr) - @intFromPtr(textinput.ptr);
		//std.debug.print("''{*} {d}\n", .{text.ptr, (@intFromPtr(text.ptr) - @intFromPtr(textinput.ptr))});
		
		//Keywords, etc...
		if (std.mem.eql(u8, "var", text)) {
			try tokenList.append(Token{.kind = .Var, .text = text});
		}
		else if (std.mem.eql(u8, "=", text)) {
			try tokenList.append(Token{.kind = .Assignment, .text = text});
		}
		else if (std.mem.eql(u8, "+", text)) {
			try tokenList.append(Token{.kind = .Addition, .text = text});
		}
		else if (std.mem.eql(u8, ";", text)) {
			try tokenList.append(Token{.kind = .EndStatement, .text = text});
		}
		else if (std.mem.eql(u8, "print", text)) {
			try tokenList.append(Token{.kind = .Print, .text = text});
		}

		//TODO: shortcut only checks if first digit is numeric
		else if (std.ascii.isDigit(text[0])) {
			try tokenList.append(Token{.kind = .Number, .text = text});
		}
		//TODO: shortcut only checks if first character is alphabetic
		else if (std.ascii.isAlphabetic(text[0])) {
			try tokenList.append(Token{.kind = .Identifier, .text = text});
		}
		else {
			std.debug.print("WTF BRO? '{s}'\n", .{text});
			return error.UnrecognizedLexicalItem;
		}
	}

	return tokenList.toOwnedSlice();
}
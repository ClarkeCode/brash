const std = @import("std");
const tokens = @import("tokens.zig");
const lexer = @import("lexer.zig");
const strutil = @import("stringutils.zig");

const PrecedenceLevel = enum {
	Default,
	Comma,
	Assignment,
	Equality,
	LogicalOr,
	LogicalAnd,
	LogicalXor,
	Relational,
	Term,
	Factor,
	Unary,
	FunctionCall,
	MemberAccess,
	Primary,

	pub fn precedenceOfTokenType(tk: tokens.TokenType) PrecedenceLevel {
		return switch (tk) {
			.Assignment => .Assignment,
			.Equality, .Inequality => .Equality,
			.Lesser, .Greater, .LesserEqual, .GreaterEqual => .Relational,
			.Add, .Subtract => .Term,
			.Multiply, .Divide, .Modulo => .Factor,
			else => .Default
		};
	}
};

//Note: this can be reworked later as a 'flat tree' to reduce cache misses
pub const ASTnode = struct {
	const Self = @This();

	token: tokens.Token,
	left: ?*Self = null,
	right: ?*Self = null,
	parent: ?*Self = null,

	pub fn init(allocator: std.mem.Allocator, token: tokens.Token) !*Self {
		var newNode = try allocator.create(Self);
		newNode.token = token;
		newNode.parent = null;
		newNode.left =  null;
		newNode.right = null;
		return newNode;
	}

	pub fn getRootNode(this: *ASTnode) *ASTnode {
		var current = this;
		while (current.parent != null) {
			current = current.parent.?;
		}
		return current;
	}

	pub fn hasHigherPrecedence(aaa: *ASTnode, other: *ASTnode) bool {
		return (
			@intFromEnum(PrecedenceLevel.precedenceOfTokenType(aaa.token.kind)) >
			@intFromEnum(PrecedenceLevel.precedenceOfTokenType(other.token.kind))
		);
	}
};
const ASTtree = ASTnode;


const ParseError = error {
	EmptyInput,
	UnexpectedToken,
};

pub const Parser = struct {
	tokens: *[]const tokens.Token,
	position: usize = 0,

	fn hasTokens(self: Parser) bool { return self.position < self.tokens.len; }
	fn currentToken(self: Parser) tokens.Token { return self.tokens.*[self.position]; }
	fn advance(self: *Parser) tokens.Token {
		const currentTok = self.currentToken();
		self.position += 1;
		return currentTok;
	}
	fn expect(self: Parser, expectedTokenType: tokens.TokenType) !tokens.Token {
		const tk = self.currentToken();
		if (!tk.isType(expectedTokenType)) {
			const stderr = std.io.getStdErr().writer();
			var errWriter = std.io.bufferedWriter(stderr).writer();
			errWriter.print("Expected {s} but received {s}\n", .{expectedTokenType, tk.toString()});
			return ParseError.UnexpectedToken;
		}
		return self.advance();
	}

	pub fn parse(parser: *Parser, allocator: std.mem.Allocator) !*ASTnode {

		if (parser.tokens.len < 1) {
			return ParseError.EmptyInput;
		}

		var currentNode: *ASTnode = try ASTnode.init(allocator, parser.advance());

		while (parser.hasTokens()) {
			var insertedTree = false;
			var latestNode: *ASTnode = try ASTnode.init(allocator, parser.advance());
			
			while (latestNode.hasHigherPrecedence(currentNode)) {
				//Latest has higher precedence than current root
				if (currentNode.parent == null) {
					latestNode.left = currentNode;
					currentNode.parent = latestNode;
					currentNode = latestNode;
					insertedTree = true;
					break;
				}

				if (currentNode.parent) |parent| {
					currentNode = parent;
				}
			}
			//Latest has lower precedence than current node
			if (!insertedTree) {
				if (currentNode.left == null) {
					currentNode.left = latestNode;
					latestNode.parent = currentNode;
				}
				else if (currentNode.right == null) {
					currentNode.right = latestNode;
					latestNode.parent = currentNode;
				}
				else unreachable;
			}
			break;
		}
		// parser.alloc.create(comptime T: type)
		return currentNode.getRootNode();
	}
};

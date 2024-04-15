const std = @import("std");

pub fn streq(a: []const u8, b: []const u8) bool {
	return std.mem.eql(u8, a, b);
}

const lexer = @import("lexer.zig");
const TokenType = lexer.TokenType;

const Colour = enum { Red, Green, Blue };

pub fn variad(first: Colour, args: anytype) !void {
	std.debug.print("BBBBBBBBBBB\n", .{});
	std.debug.print("{}\n", .{first});
	std.debug.print("{}\n", .{args});

	const ArgType = @TypeOf(args);
	const argTypeInfo = @typeInfo(ArgType);
	const fieldsInfo = argTypeInfo.Struct.fields;
	
	// const TI = @typeInfo(ArgType);
	// comptime var FieldInfo = TI.Struct.fields;
	std.debug.print("{}\n", .{ArgType});
	std.debug.print("{}\n", .{fieldsInfo.len});
	std.debug.print("{}\n", .{fieldsInfo[0]});
	std.debug.print("~{}\n", .{fieldsInfo[0].type});

	std.debug.print("-------------------\n", .{});
	inline for (0..fieldsInfo.len) |index| {
		std.debug.print("{}\n", .{index});
		std.debug.print("{}\n", .{fieldsInfo[index]});
		std.debug.print("{}\n", .{fieldsInfo[index].type});
		// std.debug.print("{?}\n", .{fieldsInfo[index]});
		// std.debug.print("{}\n", .{});
	}

	// std.debug.print("{any}\n", .{TI.Struct.fields});
	// std.debug.print("{any}\n", .{FieldInfo});
	std.debug.print("-------------------\n", .{});
}

pub fn main() !void {
	// Prints to stderr (it's a shortcut based on `std.io.getStdErr()`)
	std.debug.print("Time to be {s}!\n", .{"brash"});

	// stdout is for the actual output of your application, for example if you
	// are implementing gzip, then only the compressed bytes should be sent to
	// stdout, not any debugging messages.
	const stdout_file = std.io.getStdOut().writer();
	var bw = std.io.bufferedWriter(stdout_file);
	const stdout = bw.writer(); 


	var gpa = std.heap.GeneralPurposeAllocator(.{}){};
	var alloc = gpa.allocator();

	var lex = lexer.InitLexer(alloc, "1+2==3");
	defer lex.deinit();
	// try stdout.print("{}\n", .{lex.atEOF()});
	try stdout.print("LEXER\n", .{});
	try lexer.Tokenize(&lex);
	for (lex.tokens.items) |token| {
		try stdout.print("{s}\n", .{token.toString()});
	}

	// var aa: lexer.Token = lexer.makeToken(TokenType.Equality, "==");
	// try stdout.print("{s}\n", .{aa.toString()});
	// try stdout.print("is eq {} {}\n", .{
	// 	aa.isTypes(&[_]TokenType{.Add, .Subtract}),
	// 	aa.isTypes(&[_]TokenType{.And, .Equality})
	// });

	// try stdout.print("{}.{}\n", .{streq("aa", "aabe"[0..2]), streq("aa", "aa")});
	// try stdout.print("AAAAA.\n", .{});
	try stdout.print("Run `zig build test` to run the tests.\n", .{});

	try bw.flush(); // don't forget to flush!
}

test "simple test" {
	var list = std.ArrayList(i32).init(std.testing.allocator);
	defer list.deinit(); // try commenting this out and see if zig detects the memory leak!
	try list.append(42);
	try std.testing.expectEqual(@as(i32, 42), list.pop());
}

test "trying arraylist" {
	var list = std.ArrayList(i32).init(std.testing.allocator);
	defer list.deinit();
	try list.append(234);
	try list.append(111);
	try list.append(567);

	try std.testing.expectEqualSlices(i32, &[_]i32{234, 111, 567}, list.items);

	try std.testing.expectEqual(@as(i32, 111), list.orderedRemove(1));
	try std.testing.expectEqual(@as(i32, 567), list.getLast());
	try std.testing.expectEqual(@as(i32, 567), list.pop());
	try std.testing.expectEqual(@as(i32, 234), list.pop());
}
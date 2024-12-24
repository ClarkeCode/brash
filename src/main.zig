const std = @import("std");
const lex = @import("lexer.zig").lex;


const showLexer = true;

pub fn main() !void {
	const stdout = std.io.getStdOut().writer();
	//const stderr = std.io.getStdErr().writer(); //or std.debug.print()
	try stdout.print("Time to be {s}!\n", .{"brash"});

	var gpa = std.heap.GeneralPurposeAllocator(.{}){};
	const alloc = gpa.allocator();

	const fileinput = @embedFile("samples/01.txt");
	//try stdout.print("{s}", .{fileinput});

	const toks = try lex(alloc, fileinput);
	if (showLexer) {
		try stdout.print("----- {s} -----\n", .{"Start Lexer"});
		for (toks) |token| {
			try stdout.print("{s}\n", .{token.toSlice()});
		}
		try stdout.print("----- {s} -----\n", .{"End Lexer"});
	}

	try stdout.print("Run `zig build test` to run the tests.\n", .{});
}

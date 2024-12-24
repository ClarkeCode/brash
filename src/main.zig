const std = @import("std");
//const lexer = @import("lexer.zig");
//const parser = @import("parser.zig");

const stdout_file = std.io.getStdOut().writer();
var bw = std.io.bufferedWriter(stdout_file);
const stdout = bw.writer();

pub fn main() !void {
	// Prints to stderr (it's a shortcut based on `std.io.getStdErr()`)
	std.debug.print("Time to be {s}!\n", .{"brash"});

	// stdout is for the actual output of your application, for example if you
	// are implementing gzip, then only the compressed bytes should be sent to
	// stdout, not any debugging messages.

	//var gpa = std.heap.GeneralPurposeAllocator(.{}){};
	//const alloc = gpa.allocator();

	//var lex = lexer.Lexer.init(alloc, "1+2==3");
	//defer lex.deinit();
	//try stdout.print("LEXER\n", .{});
	//try lex.tokenize();
	//for (lex.tokens.items) |token| {
	//	try stdout.print("{s}\n", .{token.toString()});
	//}

	try stdout.print("Run `zig build test` to run the tests.\n", .{});

	try bw.flush(); // don't forget to flush!
}

const std = @import("std");

pub fn streq(a: []const u8, b: []const u8) bool {
	return std.mem.eql(u8, a, b);
}

const lexer = @import("lexer.zig");
const TokenType = lexer.TokenType;

pub fn main() !void {
	// Prints to stderr (it's a shortcut based on `std.io.getStdErr()`)
	std.debug.print("Time to be {s}!\n", .{"brash"});

	// stdout is for the actual output of your application, for example if you
	// are implementing gzip, then only the compressed bytes should be sent to
	// stdout, not any debugging messages.
	const stdout_file = std.io.getStdOut().writer();
	var bw = std.io.bufferedWriter(stdout_file);
	const stdout = bw.writer();

	var aa: lexer.Token = .{.kind = TokenType.Equality, .value = "=="};
	try stdout.print("{s}\n", .{aa.toString()});
	try stdout.print("is eq {} {}\n", .{
		aa.isType(&[_]TokenType{.Add, .Subtract}),
		aa.isType(&[_]TokenType{.And, .Equality})
	});

	try stdout.print("{}.{}\n", .{streq("aa", "be"), streq("aa", "aa")});
	try stdout.print("AAAAA.\n", .{});
	try stdout.print("Run `zig build test` to run the tests.\n", .{});

	try bw.flush(); // don't forget to flush!
}

test "simple test" {
	var list = std.ArrayList(i32).init(std.testing.allocator);
	defer list.deinit(); // try commenting this out and see if zig detects the memory leak!
	try list.append(42);
	try std.testing.expectEqual(@as(i32, 42), list.pop());
}

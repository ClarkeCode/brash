const std = @import("std");
const lexer = @import("lexer.zig");
const parser = @import("parser.zig");

const stdout_file = std.io.getStdOut().writer();
var bw = std.io.bufferedWriter(stdout_file);
const stdout = bw.writer();

fn dfsTraversal(node: *const parser.ASTnode, traversed: usize) !usize {
	const currentIndex = traversed;
	try stdout.print("n{d} [label=\"<{s}>\\n'{s}'\" shape={s}];\n", .{
		traversed, node.token.kind.toString(), node.token.text,
		switch (node.token.kind) {
			.Add, .Subtract, => "octagon",
			else => "box"
		}
	});

	var moreTravel = traversed;
	if (node.left) |child| {
		const childIndex = moreTravel + 1;
		moreTravel = try dfsTraversal(child, childIndex);
		try stdout.print("n{d} -> n{d};\n", .{currentIndex, childIndex});
	}

	if (node.right) |child| {
		const childIndex = moreTravel + 1;
		moreTravel = try dfsTraversal(child, childIndex);
		try stdout.print("n{d} -> n{d};\n", .{currentIndex, childIndex});
	}

	return moreTravel;
}

pub fn main() !void {
	// Prints to stderr (it's a shortcut based on `std.io.getStdErr()`)
	std.debug.print("Time to be {s}!\n", .{"brash"});

	// stdout is for the actual output of your application, for example if you
	// are implementing gzip, then only the compressed bytes should be sent to
	// stdout, not any debugging messages.



	var gpa = std.heap.GeneralPurposeAllocator(.{}){};
	var alloc = gpa.allocator();

	var lex = lexer.Lexer.init(alloc, "1+2==3");
	defer lex.deinit();
	// try stdout.print("{}\n", .{lex.atEOF()});
	try stdout.print("LEXER\n", .{});
	try lex.tokenize();
	for (lex.tokens.items) |token| {
		try stdout.print("{s}\n", .{token.toString()});
	}


	try stdout.print("\nPARSER\n", .{});
	//TODO: may want to consider using an arena allocator for the whole thing, as all allocations should live for the life of the program
	var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
	defer arena.deinit();
	const arenAlloc = arena.allocator();

	var prs: parser.Parser = .{.tokens = &lex.tokens.items};
	var root = try prs.parse(arenAlloc);

	//Graphviz output of the AST
	try stdout.print("digraph G {c}\n", .{'{'});
	_ = try dfsTraversal(root, 0);
	try stdout.print("{c}\n", .{'}'});

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
const std = @import("std");

pub fn findFirstInstance(slice: []const u8, item: u8) ?usize {
	for (slice, 0..) |ch, i| {
		if (ch == item) return i;
	}
	return null;
}

pub fn findFirstOfSet(searchSlice: []const u8, set: []const u8) ?usize {
	var firstIndex: ?usize = null;

	for (set) |ch| {
		if (findFirstInstance(searchSlice, ch)) |value| {
			if (firstIndex) |fnd| {
				if (fnd > value) { firstIndex = value; }
			}
			else {
				firstIndex = value;
			}
		}
	}
	return firstIndex;
}

pub fn streq(a: []const u8, b: []const u8) bool {
	return std.mem.eql(u8, a, b);
}

pub fn fuzzyEqual(a: []const u8, b: []const u8) bool {
	const minlen = @min(a.len, b.len);
	return streq(a[0..minlen], b[0..minlen]);
}

test "fuzzyEqual" {
	try std.testing.expectEqual(true, fuzzyEqual("Abc", "Abc"));
	try std.testing.expectEqual(true, fuzzyEqual("Abc", "Abcd"));
	try std.testing.expectEqual(true, fuzzyEqual("Abcd", "Abc"));
	try std.testing.expectEqual(false, fuzzyEqual("Agc", "Abc"));
	try std.testing.expectEqual(false, fuzzyEqual("Abc", "Dbc"));
	try std.testing.expectEqual(false, fuzzyEqual("Agcd", "Abc"));
}

test "streq" {
	try std.testing.expectEqual(true, streq("The quick brown fox", "The quick brown fox"));
	try std.testing.expectEqual(false, streq("The quick brown fox", "The quick brown fox!"));
	try std.testing.expectEqual(false, streq("The quick brown fox", "The quick brown ox"));
	try std.testing.expectEqual(false, streq("The quick brown fox", "The quick brown fox jumps over the lazy dog"));
}

test "findFirstInstance" {
	try std.testing.expectEqual(@as(usize, 2), findFirstInstance("09146", '1').?);
	try std.testing.expect(null == findFirstInstance("09146", '5'));
}

test "findFirstOfSet" {
	try std.testing.expect(null == findFirstOfSet("Mario Land", "123"));
	try std.testing.expectEqual(@as(usize, 1), findFirstOfSet("Mario Land", "ai").?);
}
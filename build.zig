const std = @import("std");

//See https://zig.guide/build-system/zig-build/
pub fn build(b: *std.Build) void {
	const exe = b.addExecutable(.{
		.name = "brash",
		.root_source_file = b.path("src/main.zig"),
		.target = b.standardTargetOptions(.{}),
		.optimize = b.standardOptimizeOption(.{}),
	});

	b.installArtifact(exe);


	const run_exe = b.addRunArtifact(exe);
	const run_step = b.step("run", "Run the application");
	run_step.dependOn(&run_exe.step);
}

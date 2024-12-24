#Used https://opensource.com/article/18/8/what-how-makefile as a reference
SRCDIR := src/
SRCS := $(wildcard $(SRCDIR)*.zig) 

MAKE = make --no-print-directory

FINAL_TARGET = brash

$(FINAL_TARGET): $(SRCS)
	zig build
	cp zig-out/bin/$(FINAL_TARGET) .

test:
	zig build test

clean:
	find zig-cache zig-out brash -delete

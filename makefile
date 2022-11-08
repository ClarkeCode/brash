# Used https://opensource.com/article/18/8/what-how-makefile as a reference

CC = gcc -g -Wall -Wextra -Wno-unused-parameter
COMPILERFLAGS = -c -D BRASH_ALLOW_DEBUG_OUTPUT
LINKERFLAGS = -lm

TOOLDIR := tools/

SRCDIR := src/
BUILDDIR := build/
SRCS := $(wildcard $(SRCDIR)*.c) 
OBJS := $(SRCS:$(SRCDIR)%.c=$(BUILDDIR)%.o)

MAKE = make --no-print-directory

FINAL_TARGET = brash


#Rules

all: build

build: $(SRCS)
	@mkdir -p $(BUILDDIR)
	@$(MAKE) -B generated
	@$(MAKE) $(FINAL_TARGET)

generated:
	$(TOOLDIR)gen_enum_lookups.sh $(SRCDIR)*.h > $(SRCDIR)enum_lookups.c
	$(TOOLDIR)header_generator.sh $(SRCDIR)enum_lookups.c ENUM_LOOKUPS > $(SRCDIR)enum_lookups.h

$(FINAL_TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LINKERFLAGS)
	@cp $(FINAL_TARGET) tests

$(BUILDDIR)%.o: $(SRCDIR)%.c
	$(CC) $(COMPILERFLAGS) $< -o $@

clean:
	rm -f *.out *.o $(FINAL_TARGET)
	rm -rf build
	rm -f $(SRCDIR)enum_lookups.h $(SRCDIR)enum_lookups.c

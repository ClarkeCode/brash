# Used https://opensource.com/article/18/8/what-how-makefile as a reference

CC = gcc -g -Wall -Wextra -Wno-unused-parameter
COMPILERFLAGS = -c -D BRASH_ALLOW_DEBUG_OUTPUT
LINKERFLAGS = -lm

TOOLDIR := tools/

BUILDDIR := build/
SRCS := $(wildcard *.c) 
OBJS := $(SRCS:%.c=$(BUILDDIR)%.o)

MAKE = make --no-print-directory

FINAL_TARGET = brash


#Rules

all: build

build: $(SRCS)
	@mkdir -p $(BUILDDIR)
	@$(MAKE) -B generated
	@$(MAKE) $(FINAL_TARGET)

generated:
	$(TOOLDIR)gen_enum_lookups.sh *.h > enum_lookups.c
	$(TOOLDIR)header_generator.sh enum_lookups.c ENUM_LOOKUPS > enum_lookups.h

$(FINAL_TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LINKERFLAGS)
	@cp $(FINAL_TARGET) tests

$(BUILDDIR)%.o: %.c
	$(CC) $(COMPILERFLAGS) $< -o $@

clean:
	rm -f *.out *.o $(FINAL_TARGET)
	rm -rf build
	rm -f enum_lookups.h enum_lookups.c

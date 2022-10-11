# Used https://opensource.com/article/18/8/what-how-makefile as a reference

CC = gcc -g -Wall -Wextra
COMPILERFLAGS = -c
LINKERFLAGS = -lm

BUILDDIR := build
SRCS := $(wildcard *.c) 
OBJS := $(SRCS:%.c=$(BUILDDIR)/%.o)

MAKE = make --no-print-directory

FINAL_TARGET = brash


#Rules

all: build

build: $(SRCS)
	@mkdir -p $(BUILDDIR)
	@$(MAKE) -B generated
	@$(MAKE) $(FINAL_TARGET)

generated:
	./gen_enum_lookups.sh *.h > enum_lookups.c
	./header_generator.sh enum_lookups.c ENUM_LOOKUPS > enum_lookups.h

$(FINAL_TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LINKERFLAGS)

$(BUILDDIR)/%.o: %.c
	$(CC) $(COMPILERFLAGS) $< -o $@

clean:
	rm -f *.out *.o $(FINAL_TARGET)
	rm -rf build
	rm -f enum_lookups.h enum_lookups.c

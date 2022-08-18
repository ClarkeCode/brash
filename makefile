# Used https://opensource.com/article/18/8/what-how-makefile as a reference

CC = gcc -g
COMPILERFLAGS = -c
LINKERFLAGS = -lm

SRCS := $(wildcard *.c)
OBJS := $(SRCS:%.c=%.o)

all: main

main: generated $(OBJS)
	$(CC) $(LINKERFLAGS) $(OBJS) -o $@


%.o: %.c
	$(CC) $(COMPILERFLAGS) $<

generated: lexer.c
	./header_generator.sh lexer.c BRASH_LEXER #generates lexer.h
	echo "#ifndef ENUM_LOOKUPS\n#define ENUM_LOOKUPS\n//Generated file" > enum_lookups.h
	./generate_enumeration_lookups.sh enumerations.h >> enum_lookups.h
	echo "#endif" >> enum_lookups.h

clean:
	rm -f *.out main *.o
	rm -f lexer.h enum_lookups.h

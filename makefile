# Used https://opensource.com/article/18/8/what-how-makefile as a reference

CC = gcc -g -Wall -Wextra
COMPILERFLAGS = -c
LINKERFLAGS = -lm

SRCS := $(wildcard *.c) 
OBJS := $(SRCS:%.c=%.o)

all: main

main: clean generated $(OBJS)
	$(CC) $(LINKERFLAGS) $(OBJS) -o $@


%.o: %.c
	$(CC) $(COMPILERFLAGS) $<

generated: lexer.c
	./header_generator.sh lexer.c BRASH_LEXER #generates lexer.h
	./generate_enumeration_lookups.sh enumerations.h > enum_lookups.c
	./header_generator.sh enum_lookups.c ENUM_LOOKUPS > enum_lookups.h

clean:
	rm -f *.out main *.o
	rm -f lexer.h enum_lookups.h enum_lookups.c

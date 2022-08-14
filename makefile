# Used https://opensource.com/article/18/8/what-how-makefile as a reference

CC = gcc
COMPILERFLAGS = -c
LINKERFLAGS = -lm

SRCS := $(wildcard *.c)
OBJS := $(SRCS:%.c=%.o)

all: main

main: $(OBJS)
	echo "#ifndef ENUM_LOOKUPS\n#define ENUM_LOOKUPS\n//Generated file" > enum_lookups.h
	./generate_enumeration_lookups.sh enumerations.h >> enum_lookups.h
	echo "#endif" >> enum_lookups.h
	$(CC) $(LINKERFLAGS) $(OBJS) -o $@


%.o: %.c
	$(CC) $(COMPILERFLAGS) $<


clean:
	rm -f a.out main *.o

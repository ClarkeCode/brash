# Used https://opensource.com/article/18/8/what-how-makefile as a reference

CC = gcc
COMPILERFLAGS = -c
LINKERFLAGS = -lm

SRCS := $(wildcard *.c)
OBJS := $(SRCS:%.c=%.o)

all: main

main: $(OBJS)
	$(CC) $(LINKERFLAGS) $(OBJS) -o $@


%.o: %.c
	$(CC) $(COMPILERFLAGS) $<


clean:
	rm -f a.out main *.o

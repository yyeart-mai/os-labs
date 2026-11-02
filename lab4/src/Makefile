.PHONY: all clean

CC = clang
CPPFLAGS = -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g

all: parent child

parent: parent.c common.c common.h
	$(CC) $(CPPFLAGS) $(CFLAGS) parent.c common.c -o parent

child: child.c common.c common.h
	$(CC) $(CPPFLAGS) $(CFLAGS) child.c common.c -o child

clean:
	rm -f parent child
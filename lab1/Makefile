CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11

all: parent child

parent: parent.c
	$(CC) $(CFLAGS) parent.c -o parent

child: child.c
	$(CC) $(CFLAGS) child.c -o child

clean:
	rm -f parent child *.txt

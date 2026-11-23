CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -pthread -lrt

all: parent child

parent: parent.c
	$(CC) $(CFLAGS) parent.c -o parent $(LDFLAGS)

child: child.c
	$(CC) $(CFLAGS) child.c -o child $(LDFLAGS)

clean:
	rm -f parent child *.txt

CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -pedantic

all: libp4dev-d.so

p4dev.o: p4dev.c p4dev.h
	$(CC) $(CFLAGS) -fPIC $^ -c 2>p4dev.log

p4dev_types.o: p4dev_types.c p4dev_types.h
	$(CC) $(CFLAGS) -fPIC $^ -c 2>p4dev_types.log

p4rule.o: p4rule.c p4rule.h
	$(CC) $(CFLAGS) -fPIC $^ -c 2>p4rule.log

libp4dev-d.so: p4rule.o p4dev_types.o p4dev.o
	$(CC) $(CFLAGS) -shared $^ -o $@ 2>libp4dev-d.log

clean:
	rm *.log *.o *.gch libp4dev-d.so
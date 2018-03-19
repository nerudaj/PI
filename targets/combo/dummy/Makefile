CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic -lcombo -lfdt -lm

all: libp4dev-d.so

p4dev.o: p4dev.c p4dev.h
	$(CC) $(CFLAGS) -fPIC $^ -c

p4dev_tree.o: p4dev_tree.c p4dev_tree.h
	$(CC) $(CFLAGS) -fPIC $^ -c

p4rule.o: p4rule.c p4rule.h
	$(CC) $(CFLAGS) -fPIC $^ -c

bst_lpm.o: lib/bst_lpm.c lib/bst_lpm.h
	$(CC) $(CFLAGS) -fPIC $^ -c

vector.o: lib/vector.c lib/vector.h
	$(CC) $(CFLAGS) -fPIC $^ -c

bstlpm.o: engines/bstlpm.c engines/bstlpm.h
	$(CC) $(CFLAGS) -fPIC $^ -c

cuckoo.o: engines/cuckoo.c engines/cuckoo.h
	$(CC) $(CFLAGS) -fPIC $^ -c

standard.o: engines/standard.c engines/standard.h
	$(CC) $(CFLAGS) -fPIC $^ -c

tcam.o: engines/tcam.c engines/tcam.h
	$(CC) $(CFLAGS) -fPIC $^ -c

libp4dev-d.so: p4dev.o p4dev_tree.o p4rule.o bst_lpm.o bstlpm.o vector.o cuckoo.o standard.o tcam.o
	$(CC) $(CFLAGS) -shared $^ -o $@

install-local: libp4dev-d.so
	cp $^ ../../../CLI/

install: libp4dev-d.so
	cp $^ /usr/local/lib
	ldconfig

clean:
	rm -f *.o *.so
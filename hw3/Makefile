#Sample Makefile for Malloc
CC=gcc
CFLAGS=-g -O0 -fPIC -fno-builtin
CFLAGS_AFT=-lm -lpthread

all: check

clean:
	rm -rf libmalloc.so malloc.o free.o calloc.o realloc.o

# final.o: common.o malloc.o free.o calloc.o realloc.o
# 	ld -r common.o malloc.o free.o calloc.o realloc.o final.o

libmalloc.so: malloc.o free.o calloc.o realloc.o
	$(CC) $(CFLAGS) -shared -Wl $^ -o $@

test: test.o
	$(CC) $(CFLAGS) $< -o $@

# For every XYZ.c file, generate XYZ.o.
%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

check:	libmalloc.so test
	LD_PRELOAD=`pwd`/libmalloc.so ./test

dist: clean
	dir=`basename $$PWD`; cd ..; tar cvf $$dir.tar ./$$dir; gzip $$dir.tar

default: all

all: clean ramsize.o ramsize
	./ramsize

clean:
	rm -rf ramsize ramsize.o
ramsize.o: ramsize.c
	gcc -g -c -Wall -Werror -fpic  -o ramsize.o ramsize.c

ramsize: ramsize.o
	gcc -g -o ramsize ramsize.o

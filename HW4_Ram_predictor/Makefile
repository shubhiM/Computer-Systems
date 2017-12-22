default: all

all: clean predictRam.o predictRam
	./predictRam

clean:
	rm -rf predictRam predictRam.o
predictRam.o: predictRam.c
	gcc -g -c -Wall -Werror -fpic  -o predictRam.o predictRam.c

predictRam: predictRam.o
	gcc -g -o predictRam predictRam.o

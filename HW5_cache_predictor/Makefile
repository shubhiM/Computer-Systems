default: all

all: clean tlb.o tlb
	./tlb

clean:
	rm -rf tlb tlb.o
tlb.o: tlb.c
	gcc -O0 -g -c -Wall -Werror -fpic  -o tlb.o tlb.c

tlb: tlb.o
	gcc -g -o tlb tlb.o

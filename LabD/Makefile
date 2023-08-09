all: clean compile link

compile:
	nasm -f elf32 multi.s -o multi.o

link:
	gcc -m32 -g -Wall multi.o -o multi 

clean:
	rm -f *.o all compile link
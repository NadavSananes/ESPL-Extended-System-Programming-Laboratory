all: clean start util task1

start:
	nasm -f elf32 start.s -o start.o

util:
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

task1:
	gcc -m32 -g -Wall start.o util.o -o task1 

clean:
	rm -f *.o all task1
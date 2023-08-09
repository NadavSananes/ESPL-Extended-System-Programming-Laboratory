all: clean start util main task2
start:
	nasm -f elf32 start.s -o start.o

util:
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

main:
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o


task2:
	ld -m elf_i386 start.o main.o util.o -o task2

clean:
	rm -f *.o all task2
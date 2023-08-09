all: clean task1 task4

task1:	
	gcc -m32 -g -Wall task1.c -o task1

task4:
	gcc -m32 -g -Wall -fno-pie -fno-stack-protector task4.c -o task4

clean:
	rm -f *.o task1 task4

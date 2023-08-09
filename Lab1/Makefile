all: count addresses base menu
	count addresses base

count:
	gcc -g -o count ./task3/count-words.c

addresses:
	gcc -m32 -g -Wall -o addresses ./task3/addresses.c

base:
	gcc -m32 -g -Wall -o base ./task3/base.c

menu:
	gcc -m32 -g -Wall -o menu menu_map.c
clean:
	rm count addresses base menu
all: main

main: format
	gcc main.c -Wall -Wextra -g -o main.out

format:
	clang-format -i *.c

clean:
	rm -vf *.out

all:
	gcc -Wall -Wextra -g3 -O0 -fsanitize=address,undefined my_shell.c -o my_shell

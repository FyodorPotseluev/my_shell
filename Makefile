
all:
	gcc -Wall -Wextra -g3 -O0 -fsanitize=address,undefined -D EXEC_MODE my_shell.c -o my_shell
	#gcc -Wall -Wextra -g3 -O0 -D EXEC_MODE my_shell.c -o my_shell

print_tokens:
	gcc -Wall -Wextra -g3 -O0 -fsanitize=address,undefined -D PRINT_TOKENS_MODE my_shell.c -o my_shell
	#gcc -Wall -Wextra -g3 -O0 -D PRINT_TOKENS_MODE my_shell.c -o my_shell

clean:
	rm -f ./my_shell

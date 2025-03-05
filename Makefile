
all:
	gcc -Wall -Wextra -g3 -O0 -fsanitize=address,undefined -D EXEC_MODE my_shell.c -o my_shell
	#gcc -Wall -Wextra -g3 -O0 -D EXEC_MODE my_shell.c -o my_shell

print_tokens:
	#gcc -Wall -Wextra -g3 -O0 -fsanitize=address,undefined -D PRINT_TOKENS_MODE my_shell.c -o my_shell
	gcc -Wall -Wextra -g3 -O0 -D PRINT_TOKENS_MODE my_shell.c -o my_shell

clean:
	rm -f ./my_shell

help:
	@echo "Try one of the following make goals:"
	@echo " > - (no goals) compile using sanitizers. Execute programs;"
	@echo " > print_tokens - compile using sanitizers. Print all the tokens;"
	@echo " > clean - delete the program executive file."

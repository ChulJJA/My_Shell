CC = gcc -Wall -Werror -fsanitize=address,undefined
MYSH = mysh

mysh: myshell.c
	$(CC) -o $(MYSH) myshell.c

clean: 
	rm -f $(MYSH)
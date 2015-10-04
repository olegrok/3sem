#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
int parse_to_parts(char *str, char *arg[])
{
	int counter = 0;
	char *fr_stk = strtok(str, "|");
	if (fr_stk == NULL)
		return 0;
	arg[counter] = fr_stk;
	while (fr_stk != NULL) {

		fr_stk = strtok(NULL, "|");
		arg[counter + 1] = fr_stk;
		counter++;

	}
	return counter;

}


int parse(char *str, char *arg[])
{
	int counter = 0;
	char *fr_stk = strtok(str, " \n");
	if (fr_stk == NULL)
		return -13;
	arg[counter] = fr_stk;
	if (!strncmp(arg[counter], "myexit", 6))
		return -1;
	while (fr_stk != NULL) {

		fr_stk = strtok(NULL, " \n");
		arg[counter + 1] = fr_stk;
		counter++;

	}
	return counter;

}

int main()
{
	char str[1024] = { }, str_buf[1024] ={ };	
	char *args[1024] = { };
	char *parts_array[1024] = { };
	int str_ok = 0, parts = 0, i = 0, for_fork = 0;
	int pipefd1[2] = { };
	int pipefd2[2] = { };
	while (1) {
		printf("myshell $ ");
		fgets(str, 1024, stdin);
		memcpy(str_buf, str, 1024);
		str_ok = parse(str_buf, args);		
		if (str_ok == -13)
			continue;     
		if (str_ok == -1)
			break;
		parts = parse_to_parts(str, parts_array);
		for (i = 0; i < parts; i++) {
			pipe(pipefd1);
			for_fork = fork();

			if (for_fork == 0) {
				close(pipefd1[0]);
				parse(parts_array[i], args);

				if (i != 0) {
					close(pipefd2[1]);
					dup2(pipefd2[0], 0);
				}
				if (i != (parts - 1))
					dup2(pipefd1[1], 1);
				execvp(args[0], args);
				perror("Exec");
			}
			close(pipefd1[1]);
			if (i != 0)
				close(pipefd2[0]);
			pipe(pipefd2);
			dup2(pipefd1[0], pipefd2[0]);
		}

		for (i = 0; i < parts; i++)
			wait(0);
	}

	return 0;
}

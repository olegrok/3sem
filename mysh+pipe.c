#define SIZE 4096
#define EMPTYSTR -13
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <wctype.h>
#include <errno.h>

int exit_check(const char *str)
{
	int len = strlen(str);
	int i = 0;
	for (; i < len; i++) {
		if (!isspace(str[i]))
			break;
	}
	if (i == len)
		return EMPTYSTR;
	if (str[i] == '|') {
		fputs("mybash: ошибка синтаксиса около неожиданной лексемы `|'\n", stderr);
		return EMPTYSTR;
	}
	if (i < len) {
		if (strncmp(&str[i], "myexit", 6) == 0)
			return -1;
	}
	return 0;
}

int parse(char *str, char *arg[], const char *delim)
{
	int counter = 0;
	char *fr_stk = strtok(str, delim);
	if (fr_stk == NULL)
		return counter;
	arg[counter] = fr_stk;
	while (fr_stk != NULL) {
		fr_stk = strtok(NULL, delim);
		arg[counter + 1] = fr_stk;
		counter++;
	}
	return counter;

}


int main()
{
	char str[SIZE] = { };
	char *args[SIZE] = { };
	char *parts_array[SIZE] = { };
	int str_ok = 0, parts = 0, i = 0;
	int pipefd1[2] = { };
	int pipefd2[2] = { };
	while (1) {
		printf("myshell$ ");
		if (fgets(str, SIZE, stdin) == NULL) {
			printf("\n");
			return 0;
		}
		str_ok = exit_check(str);
		if (str_ok == EMPTYSTR)
			continue;
		if (str_ok == -1)
			break;
		parts = parse(str, parts_array, "|");
		for (i = 0; i < parts; i++) {
			if (pipe(pipefd1) < 0)
				perror("Pipe failed");
			if (fork() == 0) {
				if (close(pipefd1[0]) < 0) {
					perror("Close failed");
					exit(-1);
				}
				parse(parts_array[i], args, " \n");
				if (i != 0) {
					if (dup2(pipefd2[0], 0) < 0) {
						perror("Dup2 failed");
						exit(-1);
					}
				}
				if (i != (parts - 1)) {
					if (dup2(pipefd1[1], 1) < 0) {
						perror("Dup2 failed");
						exit(-1);
					}
				} else if (close(pipefd1[1]) < 0) {
					perror("Close failed");
					exit(-1);

				}
				execvp(args[0], args);
				perror("Exec");
				return -1;
			}

			if (close(pipefd1[1]) < 0) {
				perror("Close failed");
			}
			if (i != 0)
				if (close(pipefd2[0]) < 0) {
					perror("Close failed");
				}
			if (pipe(pipefd2) < 0)
				perror("Pipe failed");
			if (close(pipefd2[1]) < 0)
				perror("Close failed");
			if (dup2(pipefd1[0], pipefd2[0]) < 0)
				perror("Dup2 failed");
			if (close(pipefd1[0]) < 0)
				perror("Close failed");
			if (i == (parts - 1)) {
				if (close(pipefd2[0]) < 0)
					perror("Close failed");
			}
		}

		for (i = 0; i < parts; i++)
			wait(0);
	}

	return 0;
}

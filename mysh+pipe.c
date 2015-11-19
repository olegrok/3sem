#define SIZE 4096
#define WRONGSTR -13
#define EMPTYSTR -12
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
	int i = 0, j = 0;
	for (; i < len; i++) {
		if (!isspace(str[i]))
			break;
	}
	if (i == len)
		return WRONGSTR;
	if (str[i] == '|') {
		fputs
		    ("mybash: ошибка синтаксиса около неожиданной лексемы `|'\n",
		     stderr);
		return WRONGSTR;
	}
	for (j = len - 1; j > i; j--) {
		if (!isspace(str[j]))
			break;
	}
	if (str[j] == '|') {
		return j + 1;
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
	char *parse_identifier = strtok(str, delim);
	if (parse_identifier == NULL)
		return counter;
	arg[counter] = parse_identifier;
	while (parse_identifier != NULL) {
		parse_identifier = strtok(NULL, delim);
		arg[counter + 1] = parse_identifier;
		counter++;
	}
	return counter;

}


int main()
{
	char str[SIZE] = { };
	char *args[SIZE] = { };
	char *parts_array[SIZE] = { };
	int parts = 0, i = 0, pointer = 0, str_ok = 0;
	int pipefd1[2] = { };
	int pipefd2[2] = { };
	while (1) {
		pointer = 0;
		str_ok = 0;
		printf("myshell$ ");
		do {
			if (str_ok)
				printf("> ");
			if (str_ok == EMPTYSTR)
				str_ok = 0;
			pointer += str_ok;
			if (fgets(&str[pointer], SIZE - pointer, stdin) ==
			    NULL) {
				printf("\n");
				return 0;
			}
			str_ok = exit_check(&str[pointer]);
			if (str_ok == EMPTYSTR) {
				if (pointer == 0) {
					break;
				}
				continue;
			}
			if (str_ok < 0)
				break;

		}
		while (str_ok != 0);
		if (str_ok == EMPTYSTR || str_ok == WRONGSTR)
			continue;
		if (str_ok == -1)
			break;
		parts = parse(str, parts_array, "|");
		for (i = 0; i < parts; i++) {
			pointer = exit_check(parts_array[i]);
			if (pointer == -1)
				return 0;
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

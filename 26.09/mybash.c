#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main()
{
	char *str = (char *) calloc(sizeof(char), 1024);
	char *fr_stk = 0;
	char *arg[1024] = { };
	char *path = 0;
	int i = 0, for_fork = 0, counter = 0;
	int len = 0;
	while (1) {
		printf("myshell $ ");
		fgets(str, 1024, stdin);
		printf("str: %s\n", str);
		fr_stk = strtok(str, " \n");
		printf("!!!!!str: %s\n", str);
		path = fr_stk;
		if (!strncmp(path, "myexit", 6))
			break;
		counter = 0;

		arg[counter] = fr_stk;

		while (fr_stk != NULL) {

			fr_stk = strtok(NULL, " \n");
			arg[counter + 1] = fr_stk;
			counter++;

		}
		printf("counter = %d\n", counter);
		for_fork = fork();
		if (for_fork < 0) {
			perror("Fork failed");
			return -1;
		}
		if (for_fork == 0) {
			printf("path: %s\n", path);
			for (i = 0; i <= counter; i++)
				printf("arg[%d]: %p, %s\n", i, arg[i],
				       arg[i]);
			printf("%p", arg);
			execvp(path, arg);
			perror("Execvp failed");
			return -1;
		} else {
			wait(0);
		}
	}
	return 0;
}

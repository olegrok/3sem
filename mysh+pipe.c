#define size 4096
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>


int parse(char *str, char *arg[], const char *delim)
{
	int counter = 0;
	char *fr_stk = strtok(str, delim);
	if (fr_stk == NULL)
		return -13;
	arg[counter] = fr_stk;
	if (!strncmp(arg[counter], "myexit", 6))
		return -1;
	while (fr_stk != NULL) {

		fr_stk = strtok(NULL, delim);
		arg[counter + 1] = fr_stk;
		counter++;

	}
	return counter;

}

int main()
{
	char str[size] = { }, str_buf[size] ={ };	
	char *args[size] = { };
	char *parts_array[size] = { };
	int str_ok = 0, parts = 0, i = 0, for_fork = 0;
	int pipefd1[2] = { };
	int pipefd2[2] = { };
	while (1) {
		printf("myshell $ ");
		fgets(str, size, stdin);
		memcpy(str_buf, str, size);
		str_ok = parse(str_buf, args, " \n");		
		if (str_ok == -13)
			continue;     
		if (str_ok == -1)
			break;
		parts = parse(str, parts_array,"|");		
		for (i = 0; i < parts; i++) {
			if(pipe(pipefd1) < 0)
				perror("Pipe failed");
			for_fork = fork();
			if(for_fork < 0)
			{
				perror("Fork failed");
				return -1;
			}
			if (for_fork == 0) {
				if(close(pipefd1[0]) < 0){
					perror("Close failed");
					exit(-1);
				}
				parse(parts_array[i], args, " \n");
				if (i != 0) {
					/*if(close(pipefd2[1]) < 0){
						perror("Close failed");											
						exit(-1);						
					}
					*/					
					if(dup2(pipefd2[0], 0) < 0){
						perror("Dup2 failed");											
						exit(-1);
					}	
				}
				if (i != (parts - 1)){
					if(dup2(pipefd1[1], 1) < 0){
						perror("Dup2 failed");											
						exit(-1);
					}
				}
				else
					if(close(pipefd1[1]) < 0){
						perror("Close failed");											
						exit(-1);
					
					}				
				execvp(args[0], args);
				perror("Exec");
				return -1;
			}
			if(close(pipefd1[1]) < 0){
					perror("Close failed");											
				}
			if (i != 0)
				if(close(pipefd2[0]) < 0){
					perror("Close failed");											
				}
			if(pipe(pipefd2) < 0){
				perror("Pipe failed");											
			}
			

			if(close(pipefd2[1]) < 0){
				perror("Close failed"); //!!!todo											
			}
					
			if(dup2(pipefd1[0], pipefd2[0]) < 0){
				perror("Dup2 failed");											
			}
			if(close(pipefd1[0]) < 0)
				perror("Close failed");
		}

		for (i = 0; i < parts; i++)
			wait(0);
	}

	return 0;
}

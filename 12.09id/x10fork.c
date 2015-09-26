#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main()
{


	int i = 0, n = 0;
	pid_t pid = 0, oldpid = 0, ppid = getpid();
	printf("ppid = %d\n", ppid);
	for (i = 0; i < 10; i++) {
		oldpid = pid;
		pid = fork();
		if (!pid) break;

	}
	if (getpid() != ppid) {
		if (i == 0) {
			printf("N = %d, pid = %d\n", i , getpid());
			return 0;
		}
		while (kill(oldpid, 0) != -1) {wait(0);}
		printf("N = %d, pid = %d\n",i , getpid());
	} else
		for (i = 0; i < 10; i++)
			wait(0);
	return 0;
}

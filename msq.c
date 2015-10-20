#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h>
struct for_cross {
	long number;
};

struct for_cross stick = { };

int judge(int msqid, long number)
{

	printf("[%d]: Judge is on start\n", getpid());
	long i = 1;
	for (; i <= number; i++) {
		if (msgrcv(msqid, &stick, 0, i, 0) < 0)
			perror("Judge's msgrcv failed on start");
	}
	printf("[%d]: Judge approve that all runners on start\n", getpid());
	
	stick.number = number + 1;
	if (msgsnd(msqid, &stick, 0, 0) < 0)
		perror("Judge's msgsnd failed");
	if (msgrcv(msqid, &stick, 0, 2 * number + 1, 0) < 0)
		perror("Judge's rcv failed on finish");
	printf("[%d]: Judge fixed finish of cross\n", getpid());
	
	exit(0);
}

int runner(int msqid, long number, long my_number)
{
	stick.number = my_number;
	printf("[%d]: Runner number is %ld on start\n", getpid(), my_number);
	if (msgsnd(msqid, &stick, 0, 0) < 0)
		perror("Runner's msgsnd failed - ready");
		
	if (msgrcv(msqid, &stick, 0, number + my_number, 0) < 0)
		perror("Runner's msgrcv failed - cross");
	printf("[%d]: Runner number %ld started to run\n", getpid(),
	       my_number);

	stick.number = my_number + number + 1;
	printf("[%d]: Runner number %ld is on finish\n", getpid(), my_number);
	if (msgsnd(msqid, &stick, 0, 0) < 0)
		perror("Runner's msgsnd failed on start");
	exit(0);
}


int main(int argc, char *argv[])
{
	if(argc < 2){
		printf("No runners\n");
		return -1;
	}
	long n = atoi(argv[1]);
	int msqid = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
	if (!fork())
		judge(msqid, n);
	long i = 1;
	for (; i <= n; i++)
		if (!fork())
			runner(msqid, n, i);
	for (i = 0; i <= n; i++)
		wait(0);


	msgctl(msqid, IPC_RMID, 0);
	printf("Msq deleted\n");



	return 0;
}

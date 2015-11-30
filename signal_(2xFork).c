#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

pid_t* send_pid = 0;
pid_t* receive_pid = 0;

int out_char = 0, digit = 8;

char get_bit(char byte, int i)
{
	byte >>= 7 - i;
	int bit = byte & 1;
	return bit;
}

void childexit(int signo)
{
	printf("\n");
	exit(0);
}

void empty(int signo)
{
};

void one(int signo)
{
	digit--;
	out_char |= 1 << digit;
	kill(*send_pid, SIGUSR1);
}

void zero(int signo)
{
	digit--;
	kill(*send_pid, SIGUSR1);
}

int send(char *path)
{
	int fd = 0;
	char buf = 0;
	struct sigaction act = { 0 };
	act.sa_handler = empty;

	sigfillset(&act.sa_mask);
	sigaction(SIGUSR1, &act, 0);

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_BLOCK, &set, 0);

	sigfillset(&set);
	sigdelset(&set, SIGUSR1);

	if ((fd = open(path, O_RDONLY)) < 0) {
		perror("Open failed");
		exit(-1);
	}

	int i = 0;

	(*send_pid) = getpid();
	while((*receive_pid) == 0);

	while (read(fd, &buf, 1)) {
		for (i = 0; i < 8; i++) {
			if (get_bit(buf, i))
				kill(*receive_pid, SIGUSR1);
			else
				kill(*receive_pid, SIGUSR2);
			sigsuspend(&set);
		}
	}

	kill(*receive_pid, SIGCHLD);
	exit(0);
}

int receive()
{
	sigset_t set;

	struct sigaction act = { 0 };

	act.sa_handler = childexit;
	sigfillset(&act.sa_mask);
	sigaction(SIGCHLD, &act, 0);

	act.sa_handler = one;
	sigfillset(&act.sa_mask);
	sigaction(SIGUSR1, &act, 0);

	act.sa_handler = zero;
	sigfillset(&act.sa_mask);
	sigaction(SIGUSR2, &act, 0);


	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigaddset(&set, SIGUSR2);
	sigaddset(&set, SIGCHLD);

	sigprocmask(SIG_BLOCK, &set, 0);

	sigfillset(&set);
	sigdelset(&set, SIGUSR1);
	sigdelset(&set, SIGUSR2);
	sigdelset(&set, SIGCHLD);

	(*receive_pid) = getpid();
	while((*send_pid) == 0);

	while (1) {
		if (digit == 0) {
			write(2, &out_char, 1);
			digit = 8;
			out_char = 0;
		}
		sigsuspend(&set);
	}

	exit(0);
}
int main(int argc, char *argv[])
{
	if (argc != 2) {
		fputs("No input file or bad format\n", stderr);
		exit(-1);
	}

	int s_pid = shmget(IPC_PRIVATE, sizeof(pid_t), IPC_CREAT | 0666);
	int r_pid = shmget(IPC_PRIVATE, sizeof(pid_t), IPC_CREAT | 0666);
	send_pid  = shmat(s_pid, 0, 0);
	receive_pid = shmat(r_pid, 0, 0);
	(*send_pid) = 0;
	(*receive_pid) = 0;

	int pid = fork();
	if (pid < 0)
		perror("Fork failed");
	if (pid == 0)
		send(argv[1]);

	pid = fork();
	if (pid < 0)
		perror("Fork failed");
	if (pid == 0)
		receive();

	wait(0);
	wait(0);
	shmdt(send_pid);
	shmdt(receive_pid);
	shmctl(s_pid, IPC_RMID, NULL);
	shmctl(r_pid, IPC_RMID, NULL);

	exit(0);
}

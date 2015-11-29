#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


int out_char = 0, digit = 8;
pid_t pid = 0;

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

void parentexit(int signo)
{
	exit(0);
}

void empty(int signo)
{
};

void one(int signo)
{
	digit--;
	out_char |= 1 << digit;
	kill(pid, SIGUSR1);
}

void zero(int signo)
{
	digit--;
	kill(pid, SIGUSR1);
}

int send(char *path)
{
	pid_t ppid = getppid();
	int fd = 0;
	char buf = 0;
	sigset_t set;
	sigemptyset(&set);
	struct sigaction act = { 0 };
	act.sa_handler = empty;
	sigfillset(&act.sa_mask);
	sigaction(SIGUSR1, &act, 0);

	sigfillset(&set);
	sigdelset(&set, SIGUSR1);

	if ((fd = open(path, O_RDONLY)) < 0) {
		perror("Open failed");
		exit(-1);
	}

	int i = 0;

	while (read(fd, &buf, 1)) {
		for (i = 0; i < 8; i++) {
			if (get_bit(buf, i))
				kill(ppid, SIGUSR1);
			else
				kill(ppid, SIGUSR2);
			sigsuspend(&set);
		}
	}
	exit(0);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fputs("No input file or bad format\n", stderr);
		exit(-1);
	}

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

	pid = fork();
	if (pid < 0)
		perror("Fork failed");
	if (pid == 0)
		send(argv[1]);

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

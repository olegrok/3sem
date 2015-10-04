#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
	int pipefd[2] = { };
	int eof = 1, i = 0;
	int for_pipe = pipe(pipefd);
	char buf[1024] = { };
	int b_counter = 0, s_counter = 0, w_counter = 0;
	if (fork() == 0) {
		close(pipefd[0]);
		dup2(pipefd[1], 1);
		execvp(argv[1], &argv[1]);

	} else {
		close(pipefd[1]);
		while (eof != 0) {
			eof = read(pipefd[0], (void *) buf, 1024);
			for (i = 0; i < eof; i++) {
				b_counter++;
				if (buf[i] != ' ' && !w_counter || i >= 1
				    && (buf[i - 1] == ' '
					|| buf[i - 1] == '\n')
				    && buf[i] != ' ')
					w_counter++;
				if (buf[i] == '\n')
					s_counter++;

			}


		}
		printf
		    ("\t%d\t%d\t%d\n",
		     s_counter, w_counter, b_counter);
		wait(0);
	}
	return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

struct msgp {
	long mtype;		/* message type, must be > 0 */
	//int mtext[1];    /* message data */
};

int main()
{
	printf("err = %d\n", errno);
	int i = 0;
	pid_t ppid = getpid(), for_fork = 0;
	int msgflg = IPC_CREAT | 0777;
	int id = msgget(IPC_PRIVATE, msgflg);
	if (id == -1) {
		printf("Error\n");
		return -1;
	}

	struct msgp *message =
	    (struct msgp *) calloc(sizeof(struct msgp), 1);
	message->mtype = 1;
	msgsnd(id, message, 0, msgflg);
	for (i = 0; i < 10; i++) {
		for_fork = fork();
		if (for_fork == -1) {
			printf("Error\n");
			return -1;
		}
		if (for_fork == 0)
			break;

	}
	if (ppid != getpid()) {
		msgrcv(id, message, 0, i + 1, msgflg);
		printf("%d %d\n", i, getpid());
		message->mtype = i + 2;
		msgsnd(id, message, 0, msgflg);
	} else {
		for (i = 0; i < 10; i++)
			wait(0);
		perror("");
		msgctl(id, IPC_RMID, 0);
		free(message);
	}

	return 0;


}

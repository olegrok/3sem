#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


struct pth_args {
	int bufsize;
	int *queue;
	int array;
	int loop;
	int producer_sum;
	int consumer_sum;
	int start;
	int end;
	int prosem;
	int consem;
	int exit;
	int max_len;
};

int mysemop(int id, int num, int op)
{
	struct sembuf sem = { num, op, 0 };
	return semop(id, &sem, 1);
}

int my2semop(int id, int num, int op1, int op2)
{
	struct sembuf sem[2];;
	sem[0].sem_num = num;
	sem[0].sem_op = 0;
	sem[0].sem_flg = 0;
	sem[1].sem_num = num;
	sem[1].sem_op = 1;
	sem[1].sem_flg = 0;
	return semop(id, sem, 2);
}

void *producer(void *arguments)
{
	struct pth_args *arg = arguments;
	int bufsize = arg->bufsize;
	int *queue = arg->queue;
	int local_sum = 0;
	int i = 0, random = 0, point = 0;
	for (i = 0; i < arg->loop; i++) {
		random = rand() % 120;
		local_sum += random;
		mysemop(arg->prosem, 0, -1);
		point = arg->start;
		arg->start++;
		arg->start %= bufsize;
		mysemop(arg->prosem, 0, 1);
		mysemop(arg->array, point, -3);
		queue[point] = random;
		mysemop(arg->array, point, -2);

	}
	mysemop(arg->prosem, 0, -1);
	arg->producer_sum += local_sum;
	mysemop(arg->prosem, 0, 1);
	return 0;
}


void *consumer(void *arguments)
{
	struct pth_args *arg = arguments;
	int bufsize = arg->bufsize;
	int *queue = arg->queue;
	int local_sum = 0;
	int random = 0, point = 0;

	while (1) {
		mysemop(arg->consem, 0, -1);
		point = arg->end;
		arg->end++;
		arg->end %= bufsize;
		mysemop(arg->prosem, 0, -1);
		if(arg->max_len < abs(arg->end - arg->start))
			arg->max_len = abs(arg->end - arg->start);
		mysemop(arg->prosem, 0, 1);
		mysemop(arg->consem, 0, 1);

		if (my2semop(arg->array, point, 0, 1) < 0){
			mysemop(arg->array, point, -1);
			break;
		}

		random = queue[point];
		queue[point] = 0;
		mysemop(arg->array, point, 4);
		local_sum += random;
	}
	while(1) {
		mysemop(arg->prosem, 0, -1);
		point = arg->end;
		arg->end++;
		arg->end %= bufsize;
		if(arg->end == arg->start){
			arg->end--;
			local_sum += queue[point];
			queue[point] = 0;
			mysemop(arg->prosem, 0, 1);
			break;
		}
		mysemop(arg->prosem, 0, 1);
		local_sum += queue[point];
		queue[point] = 0;

	}
	mysemop(arg->consem, 0, -1);
	arg->consumer_sum += local_sum;
	mysemop(arg->consem, 0, 1);
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc < 5)
	{
		write(2, "Error\n", 7);
		return -1;
	}
	struct pth_args *arg = calloc(1, sizeof(struct pth_args));
	int producer_s = atoi(argv[1]);
	int consumer_s = atoi(argv[2]);
	arg->bufsize = atoi(argv[3]);
	arg->loop = atoi(argv[4]);
	void *res = 0;
	int i = 0;
	arg->queue = calloc(sizeof(int), arg->bufsize);
	arg->array = semget(IPC_PRIVATE, arg->bufsize, IPC_CREAT | 0777);
	if (arg->array < 0) {
		perror("semget failed");
		return -1;
	}
	for (i = 0; i < arg->bufsize; i++) {
		if (mysemop(arg->array, i, 5) < 0) {
			perror("semop failed");
			return -1;
		}
	}

	arg->prosem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0777);
	arg->consem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0777);
	mysemop(arg->consem, 0, 1);
	mysemop(arg->prosem, 0, 1);
	pthread_t *pth_producer = calloc(sizeof(pthread_t), producer_s);
	pthread_t *pth_consumer = calloc(sizeof(pthread_t), consumer_s);
	for (i = 0; i < producer_s; i++)
		if (pthread_create(&pth_producer[i], NULL, &producer, arg)
		    < 0) {
			perror("thread_create");
			return -1;
		}

	for (i = 0; i < consumer_s; i++) {
		if (pthread_create(&pth_consumer[i], NULL, &consumer, arg)
		    < 0) {
			perror("thread_create");
			return -1;
		}
	}
	for (i = 0; i < producer_s; i++)
		pthread_join(pth_producer[i], &res);
	for(i = 0; i < arg->bufsize; i++)
		while(semctl(arg->array, i, GETVAL) != 5);
	arg->end = arg->start;
	semctl(arg->array, 0, IPC_RMID);
	for (i = 0; i < consumer_s; i++){
		pthread_join(pth_consumer[i], &res);
	}
	printf("max queue len = %d\n", arg->max_len + 1);
	printf("producer_sum = %d\n", arg->producer_sum);
	printf("consumer_sum = %d\n", arg->consumer_sum);
	if (arg->producer_sum == arg->consumer_sum)
		printf("Success!\n");
	else
		printf("Fail!\n");
	free(arg->queue);
	free(arg);
	free(pth_producer);
	free(pth_consumer);
	semctl(arg->consem, 1, IPC_RMID);
	semctl(arg->prosem, 1, IPC_RMID);
	return 0;
}

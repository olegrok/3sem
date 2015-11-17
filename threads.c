#define _LOCK pthread_mutex_lock
#define _UNLOCK pthread_mutex_unlock

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

struct pth_args {
	int bufsize;
	char *queue;
	pthread_mutex_t mutex;
	pthread_cond_t cond_read;
	pthread_cond_t cond_write;
};


int begin_reader = 0;
int end_reader = 0;
int begin_writer = 0;
int end_writer = 0;
int last_message = 0;
int first_message = 0;

int portion = 5;

pthread_mutex_t check = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cmutex_read = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cmutex_write = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cmutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_read = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_write = PTHREAD_COND_INITIALIZER;

int write_in_queue(char *queue, int bufsize, char *message, int start,
		   int end, int message_size)
{

	int i = 0;
	int len = 0;
	if (end < start)
		end += bufsize;
	if (message_size < end - start)
		len = message_size;
	else
		len = end - start + 1;
	//end %= bufsize;
	 /*	printf("bufsize = %d\n", bufsize);
	   	printf("start = %d\n", start);
	   	printf("end = %d\n", end);
	   	printf("message_size = %d\n", message_size);
	   	printf("len = %d\n", len);
	   	printf("message = %s\n", message);
	*/
	for (; i < len; i++) {
		queue[(start + i) % bufsize] = message[i];
		//printf("queue[%d] = %c\n", (start + i) % bufsize, queue[(start + i) % bufsize]);
	}
	return len;
}

int read_from_queue(char *queue, int bufsize, int start, int end)
{
	int flag = 0;
	//printf("\n");
	//printf("READ_FROM_QUEUE\n");
	if (end < start) {
		//printf("write from %d to %d\n", start, bufsize);
		write(1, &queue[start], (bufsize - start + 1) % bufsize);
		//printf("\nwrite from %d to %d\n", 0, end);
		flag = write(1, &queue[0], end + 1);
	} else {
		//printf("write from %d to %d\n", start, end);
		flag = write(1, &queue[start], end - start + 1);
	}
	return flag;
}

void *reader(void *arg)
{
	//printf("Reader is created:\n");
	struct pth_args *arguments = arg;
	char *queue = arguments->queue;
	int bufsize = arguments->bufsize;
	//printf("bufsize = %d\n", bufsize);
	char *reader_buffer = calloc(1, portion);
	int j = 0, flag = 0;

	while ((flag = read(0, reader_buffer, portion)) > 0) {
		//printf("reader: read %d = %s\n", flag, reader_buffer);

		for (j = 0; j < flag;) {
			_LOCK(&cmutex);
			//printf("reader: mutex lock\n");

			while ((end_reader + 1) % bufsize == begin_writer) {
			//	printf("reader: in condition\n");
				pthread_cond_wait(&cond_write, &cmutex);
			}

			//printf("reader: leave condition\n");
			end_reader = (begin_writer + bufsize - 1);

			//printf
			//    ("flag = %d\nj = %d\nend_reader = %d\nbegin_reader = %d\n",
			//     flag, j, end_reader, begin_reader);
			if (flag - j <= end_reader - begin_reader)
				end_reader = begin_writer + flag - j - 1;
			end_reader %= bufsize;
			//printf
			//   ("flag = %d\nj = %d\nend_reader = %d\nbegin_reader = %d\n",
			//     flag, j, end_reader, begin_reader);

			//printf("begin_reader = %d\n", begin_reader);
			//printf("end_reader = %d\n", end_reader);

			//printf("reader: mutex unlock\n");
			_UNLOCK(&cmutex);

			j += write_in_queue(queue, bufsize,
					    &reader_buffer[j],
					    begin_reader, end_reader,
					    flag - j);

			_LOCK(&cmutex);
			begin_reader = (end_reader + 1) % bufsize;
			printf("end_reader = %d\nbegin_reader = %d\n", end_reader, begin_reader);
			pthread_cond_signal(&cond_read);
			_UNLOCK(&cmutex);
		}
	}
	//printf("Loop exit\n");

	_LOCK(&cmutex);
	_LOCK(&check);
	last_message++;
	_UNLOCK(&check);
	//pthread_cond_wait(&cond_write, &cmutex);
	//begin_reader += 1;
	//begin_reader %= bufsize;
	pthread_cond_signal(&cond_read);
	_UNLOCK(&cmutex);
	free(reader_buffer);
	printf("exit:\n end_reader = %d\nbegin_reader = %d\n", end_reader, begin_reader);
	return 0;
}


void *writer(void *arguments)
{
	struct pth_args *arg = arguments;
	int bufsize = arg->bufsize;
	char *queue = arg->queue;
	_LOCK(&cmutex);
	pthread_cond_wait(&cond_read, &cmutex);
	_UNLOCK(&cmutex);
	while (1) {
		_LOCK(&cmutex);
		//printf("writer: mutex lock\n");

		while ((end_writer + 1 + bufsize) % bufsize == begin_reader) {
			//printf("writer: in condition\n");
			pthread_cond_wait(&cond_read, &cmutex);
		}
		printf("writer: leave condition\n");

		_LOCK(&check);
		if (last_message
		    && (end_reader + 1) % bufsize == begin_writer) {
		//	printf("last_message\n");
			break;
		}
		_UNLOCK(&check);
		if(begin_writer != end_writer)
			begin_writer = (end_writer + 1) % bufsize;
		end_writer = (begin_reader + bufsize - 1) % bufsize;
		//printf("end_writer = %d\n", end_writer);
		//printf("writer: mutex unlock\n");
		printf("end_writer = %d\nbegin_writer = %d\n", end_writer, begin_writer);
		_UNLOCK(&cmutex);

		if (last_message && end_reader == end_writer){
			//printf("last_message:\n");
			//printf("end_writer = %d\nbegin_writer = %d\n", end_writer, begin_writer);
			break;
		}

		if (read_from_queue
		    (queue, bufsize, begin_writer, end_writer) < 0)
			break;

		if (last_message){
			printf("last_message:\n");
			printf("end_writer = %d\nbegin_writer = %d\n", end_writer, begin_writer);
		}

		_LOCK(&cmutex);
		printf("end_writer = %d\nbegin_writer = %d\n", end_writer, begin_writer);
		if (last_message
		    && end_reader == begin_writer) {
		//	printf("last_message\n");
			break;
		}
		//begin_reader = (begin_reader - 1 + bufsize) % bufsize;
		pthread_cond_signal(&cond_write);
		_UNLOCK(&cmutex);

	}

	_UNLOCK(&check);
	return 0;
}

int main(int argc, char *argv[])
{
	struct pth_args *arguments = calloc(1, sizeof(struct pth_args));
	void *res;
	if (argc < 2) {
		printf("error\n");
		return -1;
	}
	int thread_ok = 0;
	arguments->bufsize = atoi(argv[1]);
	arguments->queue = calloc(1, arguments->bufsize);
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	arguments->mutex = mutex;
	pthread_t pth_reader = 0;
	pthread_t pth_writer = 0;
	thread_ok = pthread_create(&pth_reader, NULL, &reader, arguments);
	if (thread_ok < 0) {
		perror("thread_create");
		return -1;
	}
	thread_ok = pthread_create(&pth_writer, NULL, &writer, arguments);
	if (thread_ok < 0) {
		perror("thread_create");
		return -1;
	}
	pthread_join(pth_reader, &res);
	pthread_join(pth_writer, &res);
	free(arguments->queue);
	free(arguments);
	return 0;
}

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

int portion = 7;

int start = 0;
int end = 0;

pthread_mutex_t check = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cmutex_read = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cmutex_write = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cmutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_read = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_write = PTHREAD_COND_INITIALIZER;

void *reader(void *arg)
{
	//printf("Reader is created:\n");
	struct pth_args *arguments = arg;
	char *queue = arguments->queue;
	int bufsize = arguments->bufsize;
	//printf("bufsize = %d\n", bufsize);
	char *reader_buffer = calloc(1, portion);
	int j = 0, flag = 0, i = 0;

	int tstart = 0, tend = 0;

	while ((flag = read(0, reader_buffer, portion)) > 0) {
//		printf("reader: read %d = %s\n", flag, reader_buffer);
			_LOCK(&cmutex);
			//printf("reader: mutex lock\n");
			tend = (start + bufsize - 1) % bufsize;
			tstart = j;
			_UNLOCK(&cmutex);

			for(i = 0; i < flag; i++)
			{
				//printf("READER: i = %d 		end = %d\n", (i + tstart) % bufsize, tend);
				if((i + tstart) % bufsize == tend){
					_LOCK(&cmutex);
					end = tend;
//					printf("new end = %d\n", end);
					pthread_cond_signal(&cond_write);
					while((end + 1) % bufsize == start){
//						printf("start = %d   end = %d\n", start, end);
						pthread_cond_wait(&cond_read, &cmutex);
					}

					tend = start;
					_UNLOCK(&cmutex);
				}
				queue[(i + tstart) % bufsize] = reader_buffer[i];
				//printf("RDqueue[%d] = %c\n", (i + tstart) % bufsize, queue[(i + tstart) % bufsize]);
			}
//			printf("i = %dend = %d\n", (i + tstart) % bufsize, tend);
			_LOCK(&cmutex);
			j = (i + tstart) % bufsize;
			end = (i + tstart + bufsize) % bufsize;
			pthread_cond_signal(&cond_write);
			_UNLOCK(&cmutex);
	}
	//printf("Loop exit\n");

	_LOCK(&cmutex);
	//pthread_cond_wait(&cond_write, &cmutex);
	_LOCK(&check);
	last_message++;
	_UNLOCK(&check);
	//printf("Pre_end_cond: end = %d", end);
	pthread_cond_signal(&cond_read);
	_UNLOCK(&cmutex);
	free(reader_buffer);
	return 0;
}


void *writer(void *arguments)
{
	struct pth_args *arg = arguments;
	int bufsize = arg->bufsize;
	char *queue = arg->queue;
	int tend = 0, tstart = 0;
	int i = 0;
	char *writer_buffer = calloc(1, bufsize);
	//printf("Writer create\n");
	//while (1) {
		_LOCK(&cmutex);
		tstart = 0;
		tend = end;
		_UNLOCK(&cmutex);

		for(; ; i++){
			_LOCK(&check);
			if(last_message && start == end){
				printf("W: last_message1\n");
				break;
			}
			_UNLOCK(&check);

//			printf("WRITE: tstart = %d  	tend = %d\n", tstart, tend);
			if((tstart + i) % bufsize == tend){
				write(1, writer_buffer, i);
				_LOCK(&cmutex);
				start = (tstart + i) % bufsize;
				pthread_cond_signal(&cond_read);
				if(last_message && start == end)
					break;
				while(start == end){
					//printf("Wcond: start = %d end = %d\n", start, end);
					pthread_cond_wait(&cond_write, &cmutex);
				}
				tend = end;
				tstart = start;
				_UNLOCK(&cmutex);
				i = -1;
				continue;
			}
			//printf("i%dWRqueue[%d] = %c\n", i,(tstart + i) % bufsize, queue[(tstart + i) % bufsize]);
			writer_buffer[i % bufsize] = queue[(tstart + i) % bufsize];
			if((i + 1) % bufsize == 0){
				write(1, writer_buffer, bufsize - 1);
				_LOCK(&cmutex);
				start += (bufsize - 1);
				start %= bufsize;
				tstart = start;
				tend = (end + bufsize - 1) % bufsize;
				pthread_cond_signal(&cond_read);
				_UNLOCK(&cmutex);
				i = -1;
				continue;
			}
		}

	free(writer_buffer);
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

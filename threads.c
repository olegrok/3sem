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
	int* reader_size;
	pthread_mutex_t mutex;
	pthread_mutex_t check;
	pthread_cond_t* cond_read;
	pthread_cond_t* cond_write;
	int *last_message;
	int *start;
	int *end;
};


//int last_message = 0;

void *reader(void *arguments)
{
	struct pth_args *arg = arguments;
	int bufsize = arg->bufsize;
	char *queue = arg->queue;
	pthread_mutex_t mutex = arg -> mutex;
	pthread_mutex_t check = arg -> check;

	pthread_cond_t* cond_read = arg -> cond_read;
	pthread_cond_t* cond_write = arg -> cond_write;

	int* start = arg->start;
	int* end = arg->end;

	int* reader_size = arg->reader_size;

	int* last_message = arg->last_message;

	char *reader_buffer = calloc(1, (*reader_size) + 1);
	int counter = 0, flag = 0, i = 0;

	int tstart = 0, tend = 0;

	while ((flag = read(0, reader_buffer, (*reader_size))) > 0) {
//              printf("reader: read %d = %s\n", flag, reader_buffer);

		_LOCK(&mutex);
		//printf("reader: mutex lock\n");
		tend = ((*start) + bufsize - 1) % bufsize;
		tstart = counter;
		_UNLOCK(&mutex);

		for (i = 0; i < flag; i++) {
			//printf("READER: i = %d                end = %d\n", (i + tstart) % bufsize, tend);
			if ((i + tstart) % bufsize == tend) {
				_LOCK(&mutex);
				(*end) = tend;
				pthread_cond_signal(cond_write);
				while (((*end) + 1) % bufsize == (*start)) {
//		            printf("start = %d   end = %d\n", *start, *end);
					pthread_cond_wait(cond_read,
							  &mutex);
				}
				tend = ((*start) + bufsize - 1) % bufsize;	//?
				_UNLOCK(&mutex);
			}

			queue[(i + tstart) % bufsize] = reader_buffer[i];
			//printf("RDqueue[%d] = %c\n", (i + tstart) % bufsize, queue[(i + tstart) % bufsize]);
		}
//                      printf("i = %dend = %d\n", (i + tstart) % bufsize, tend);
		_LOCK(&mutex);
		counter = (i + tstart) % bufsize;
		(*end) = (i + tstart) % bufsize;
		pthread_cond_signal(cond_write);
		_UNLOCK(&mutex);
	}
	//printf("Loop exit\n");

	_LOCK(&mutex);
	_LOCK(&check);
	(*last_message)++;
	_UNLOCK(&check);
	//printf("Pre_end_cond: end = %d", end);
	pthread_cond_signal(cond_write);
	_UNLOCK(&mutex);
	free(reader_buffer);
	return 0;
}


void *writer(void *arguments)
{
	struct pth_args *arg = arguments;
	int bufsize = arg->bufsize;
	char *queue = arg->queue;
	pthread_mutex_t mutex = arg -> mutex;
	pthread_mutex_t check = arg -> check;

	pthread_cond_t* cond_read = arg -> cond_read;
	pthread_cond_t* cond_write = arg -> cond_write;
	int* start = arg->start;
	int* end = arg->end;

	int* last_message = arg->last_message;

	int tend = 0, tstart = 0;
	int i = 0;
	char *writer_buffer = calloc(1, bufsize);
	//printf("Writer create\n");
	_LOCK(&mutex);
	tend = *end;
	_UNLOCK(&mutex);

	for (;; i++) {

		_LOCK(&check);
		if ((*last_message) && (*start) == (*end)) {
			break;
		}
		_UNLOCK(&check);

//                      printf("WRITE: tstart = %d      tend = %d\n", tstart, tend);
		if ((tstart + i) % bufsize == tend) {
			write(1, writer_buffer, i);
			_LOCK(&mutex);
			(*start) = (tstart + i) % bufsize;
			pthread_cond_signal(cond_read);
			while ((*start) == (*end)) {
				_LOCK(&check);
				//printf("Wcond: start = %d end = %d\n last_message = %d", *start, *end, *last_message);
				if ((*last_message) && (*start) == (*end)){
					_UNLOCK(&check);
					break;
				}
				_UNLOCK(&check);
				pthread_cond_wait(cond_write, &mutex);
			}
			tend = *end;
			tstart = *start;
			_UNLOCK(&mutex);
			i = -1;
			continue;
		}
		//printf("i%dWRqueue[%d] = %c\n", i,(tstart + i) % bufsize, queue[(tstart + i) % bufsize]);
		writer_buffer[i % bufsize] = queue[(tstart + i) % bufsize];
		if ((i + 1) % bufsize == 0) {
			write(1, writer_buffer, bufsize - 1);
			_LOCK(&mutex);
			(*start) += (bufsize - 1);
			(*start) %= bufsize;
			tstart = (*start);
			tend = ((*end) + bufsize - 1) % bufsize;
			pthread_cond_signal(cond_read);
			_UNLOCK(&mutex);
			i = -1;
			continue;
		}
	}
	free(writer_buffer);
	return 0;
}

int main(int argc, char *argv[])
{
	int bufsize = 0;
	int reader_size = 0;
	struct pth_args *arg = calloc(1, sizeof(struct pth_args));
	void *res = 0;

	int opt = 0;
	while((opt = getopt(argc, argv, "b:n:")) != -1){
		switch(opt){
			case 'b':
				bufsize  = atoi(optarg);
				if(bufsize < 2){
					fputs("The buffer size must be greater than 1\n", stderr);
					return -1;
				}
				break;
			case 'n':
				reader_size = atoi(optarg);
				if(reader_size < 1){
					fputs("The reader size must be greater than 0\n", stderr);
					return -1;
				}
				break;
			default:
				fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                           argv[0]);
				return -1;
		}

	}

	if(bufsize == 0)
		bufsize = 64;
	if(reader_size == 0)
		reader_size = 64;


	int thread_ok = 0;
	arg->bufsize = bufsize;
	arg->queue = calloc(1, arg->bufsize);
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t check = PTHREAD_MUTEX_INITIALIZER;
	arg->mutex = mutex;
	arg->check = check;

	pthread_cond_t cond_read = PTHREAD_COND_INITIALIZER;
	pthread_cond_t cond_write = PTHREAD_COND_INITIALIZER;
	arg->cond_write = &cond_write;
	arg->cond_read = &cond_read;

	int start = 0;
	int end = 0;
	arg->start = &start;
	arg->end = &end;

	//reader_size = 4;
	arg->reader_size = &reader_size;

	int last_message = 0;
	arg->last_message = &last_message;

	pthread_t pth_reader = 0;
	pthread_t pth_writer = 0;
	thread_ok = pthread_create(&pth_reader, NULL, &reader, arg);
	if (thread_ok < 0) {
		perror("thread_create");
		return -1;
	}
	thread_ok = pthread_create(&pth_writer, NULL, &writer, arg);
	if (thread_ok < 0) {
		perror("thread_create");
		return -1;
	}
	pthread_join(pth_reader, &res);
	pthread_join(pth_writer, &res);
	free(arg->queue);
	free(arg);
	return 0;
}

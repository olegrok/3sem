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
	int reader_size;
	pthread_mutex_t* mutex;
	pthread_mutex_t* check;
	pthread_cond_t *cond_read;
	pthread_cond_t *cond_write;
	int last_message;
	int start;
	int end;
};

void *reader(void *arguments)
{
	struct pth_args *arg = arguments;
	int bufsize = arg->bufsize;
	char *queue = arg->queue;

	char *reader_buffer = calloc(1, arg->reader_size + 1);
	int counter = 0, flag = 0, i = 0;
	int tstart = 0, tend = 0;

	while ((flag = read(0, reader_buffer, arg->reader_size)) > 0) {
		_LOCK(arg->mutex);
		//printf("reader: mutex lock\n");
		tend = (arg->start + bufsize - 1) % bufsize;
		tstart = counter;
		_UNLOCK(arg->mutex);

		for (i = 0; i < flag; i++) {
			if ((i + tstart) % bufsize == tend) {
				_LOCK(arg->mutex);
				arg->end = tend;
				pthread_cond_signal(arg->cond_write);
				while ((arg->end + 1) % bufsize == arg->start) {
//                          printf("start = %d   end = %d\n", *start, *end);
					pthread_cond_wait(arg->cond_read,
							  arg->mutex);
				}
				tend = (arg->start + bufsize - 1) % bufsize;
				_UNLOCK(arg->mutex);
			}

			queue[(i + tstart) % bufsize] = reader_buffer[i];
			//printf("RDqueue[%d] = %c\n", (i + tstart) % bufsize, queue[(i + tstart) % bufsize]);
		}
//                      printf("i = %dend = %d\n", (i + tstart) % bufsize, tend);
		_LOCK(arg->mutex);
		counter = (i + tstart) % bufsize;
		arg->end = (i + tstart) % bufsize;
		pthread_cond_signal(arg->cond_write);
		_UNLOCK(arg->mutex);
	}

	_LOCK(arg->mutex);
	_LOCK(arg->check);
	arg->last_message++;
	_UNLOCK(arg->check);
	pthread_cond_signal(arg->cond_write);
	_UNLOCK(arg->mutex);
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
	_LOCK(arg->mutex);
	tend = arg->end;
	_UNLOCK(arg->mutex);

	while (1) {

		_LOCK(arg->check);
		if (arg->last_message && (arg->start == arg->end)) {
			break;
		}
		_UNLOCK(arg->check);

//                      printf("WRITE: tstart = %d      tend = %d\n", tstart, tend);
		if ((tstart + i) % bufsize == tend) {
			write(1, writer_buffer, i);
			_LOCK(arg->mutex);
			arg->start = (tstart + i) % bufsize;
			pthread_cond_signal(arg->cond_read);
			while (arg->start == arg->end) {
				_LOCK(arg->check);
				//printf("Wcond: start = %d end = %d\n last_message = %d", *start, *end, last_message);
				if (arg->last_message && (arg->start == arg->end)) {
					_UNLOCK(arg->check);
					break;
				}
				_UNLOCK(arg->check);
				pthread_cond_wait(arg->cond_write, arg->mutex);
			}
			tend = arg->end;
			tstart = arg->start;
			_UNLOCK(arg->mutex);
			i = 0;
			continue;
		}
		//printf("i%dWRqueue[%d] = %c\n", i,(tstart + i) % bufsize, queue[(tstart + i) % bufsize]);
		writer_buffer[i % bufsize] = queue[(tstart + i) % bufsize];
		if ((i + 1) % bufsize == 0) {
			write(1, writer_buffer, bufsize - 1);
			_LOCK(arg->mutex);
			arg->start += (bufsize - 1);
			arg->start %= bufsize;
			tstart = arg->start;
			tend = (arg->end + bufsize - 1) % bufsize;
			pthread_cond_signal(arg->cond_read);
			_UNLOCK(arg->mutex);
			i = 0;
			continue;
		}
		i++;
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
	while ((opt = getopt(argc, argv, "b:n:")) != -1) {
		switch (opt) {
		case 'b':
			bufsize = atoi(optarg);
			if (bufsize < 2) {
				fputs
				    ("The buffer size must be greater than 1\n",
				     stderr);
				return -1;
			}
			break;
		case 'n':
			reader_size = atoi(optarg);
			if (reader_size < 1) {
				fputs
				    ("The reader size must be greater than 0\n",
				     stderr);
				return -1;
			}
			break;
		default:
			fprintf(stderr, "Usage: %s [-key] [-value]\n",
				argv[0]);
			return -1;
		}

	}

	if (bufsize == 0)
		bufsize = 64;
	if (reader_size == 0)
		reader_size = 64;


	int thread_ok = 0;
	arg->bufsize = bufsize;
	arg->queue = calloc(1, arg->bufsize);
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t check = PTHREAD_MUTEX_INITIALIZER;
	arg->mutex = &mutex;
	arg->check = &check;

	pthread_cond_t cond_read = PTHREAD_COND_INITIALIZER;
	pthread_cond_t cond_write = PTHREAD_COND_INITIALIZER;
	arg->cond_write = &cond_write;
	arg->cond_read = &cond_read;

	int start = 0;
	int end = 0;
	arg->start = start;
	arg->end = end;

	arg->reader_size = reader_size;

	int last_message = 0;
	arg->last_message = last_message;

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

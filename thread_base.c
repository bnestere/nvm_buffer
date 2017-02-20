#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <unistd.h>

#define NO_THREADS 2
#define SIZE 16384
#define REPS 1000
#define BLOCK (SIZE / NO_THREADS)

long *dat;

void subroutine1(long tid) {
	int i, j;
	long holder;

	int start = BLOCK * tid;
	int end = start + BLOCK - 1;

	for(i = 0; i < REPS; i++) {
		for(j = start; j < end; j++) {
			holder = dat[j];
			holder = holder + 1;
			dat[j] = holder;
		}
	}
}

void *DoAccess(void *_tid) {
	long tid, val;
	tid = (long) _tid;

	printf("tid %lu starting subroutine1\n", tid);
	subroutine1(tid);
	printf("tid %lu finished subroutine1\n", tid);
}

int main(int argc, char *argv[]) {

	pthread_t threads[NO_THREADS];

	int rc, i;
	long t;

	dat = malloc(sizeof(long) * SIZE);
	for(i = 0; i < SIZE; i++) {
		dat[i] = 0;
	}

	for(t=0; t < NO_THREADS; t++) {
		rc = pthread_create(&threads[t], NULL, DoAccess, (void *) t);
		if(rc) {
			printf("Pthread couldn't be made\n");
			exit(-1);
		}
	}

	for(t=0; t < NO_THREADS; t++) {
		pthread_join(threads[t], NULL);
	}
	
	return 0;
}

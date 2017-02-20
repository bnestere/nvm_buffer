#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <unistd.h>

#include "mmp_user.h"
#include "mmp_init.h"

#define NO_THREADS 2
#define SIZE 16384
#define REPS 1000
#define BLOCK (SIZE / NO_THREADS)
#define READS_PER_WRITE 1 // 7 reads per 1 write

long *dat;
rt_mem_t *rt_mem;

void subroutine1(long tid) {
	int i, j, k;
	long holders[READS_PER_WRITE], sum;

	int start = BLOCK * tid;
	int end = start + BLOCK - 1;

	for(i = 0; i < REPS; i++) {
		for(j = start; j < end; j += (READS_PER_WRITE + 2)) {
      for(k = 0; k < READS_PER_WRITE; k++) {
        holders[k] = *((long *) rt_mem->read(&dat[j + k]));
      }
      sum = 0;
      for(k = 0; k < READS_PER_WRITE; k++) {
        sum += holders[k];
      }
			//holder = *((long *) rt_mem->read(&dat[j]));
			//holder = holder + 1;
			rt_mem->write_literal((void *) sum, sizeof(long), &dat[j + READS_PER_WRITE + 1]);
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

	rt_mem = get_mmp_initializer()->initialize();
	
	dat = malloc(sizeof(long) * SIZE);
	for(i = 0; i < SIZE; i++) {
		dat[i] = 1;
	}

	//rt_mem->write_literal((void *) 15l, sizeof(long), dat);
  //kkkkkkkkkkkkkkkkkk
	//*dat = 3;

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

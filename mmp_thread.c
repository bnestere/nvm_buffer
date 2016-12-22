#include <pthread.h>
#include <time.h>

#include "mmp_user.h"

void *LoopTransfer(void *rt_mem_p) {
  rt_mem_t *rt_mem = (rt_mem_t *) rt_mem_p;
  struct timespec sleep_spec;
  sleep_spec.tv_sec = 0;
  sleep_spec.tv_nsec = 50000000;
  while(1) {
    //Transfer every 50 milliseconds for dram shenanigans
    printf("Pre transfer\n");
    rt_mem->do_transfer();
    printf("Post transfer\n");
    nanosleep(&sleep_spec, NULL);
  }

  pthread_exit(NULL);
}

#include <pthread.h>

#include "mmp_init.h"
#include "mmp_user.h"
#include "mmp_thread.h"

static rt_mem_t *rt_mem;

rt_mem_t *my_init() {
  
  pthread_t thread;
  int rc;

  rt_mem = get_rt_mem();


  rc = pthread_create(&thread, NULL, LoopTransfer, (void *) rt_mem);
  if(rc) {
    perror("Failed to create thread for transfering dram to nvm");
  }

  return rt_mem;
}

static mmp_initializer_t mmp_initializer = {
  .initialize = my_init
};

mmp_initializer_t *get_mmp_initializer() {
  return &mmp_initializer;
}

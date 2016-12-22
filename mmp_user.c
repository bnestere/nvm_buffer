#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <malloc.h>
#include <sys/mman.h>
#include <string.h>

#include "mmp_user.h"

static int is_initialized = 0, initializing = 0, transferring = 0;
static buffer_t buffer;

void internal_init() {
  printf("initializing\n");
  if(!is_initialized) {
    int r = __sync_bool_compare_and_swap(&initializing, 0, 1);
    if(r) {
      //This thread updated initialization var, so continue with idxs
      buffer.read_idx = 0;
      buffer.write_idx = 0;
      buffer.and_seed = WRITE_BUFFER_SIZE - 1;
      is_initialized = 1;
    }
  }
}

void my_write(void *data, int len, void *location) {
  int wIdx;
  printf("in my write\n");

  wIdx = __sync_fetch_and_add(&(buffer.write_idx), 1);

  write_t *ele = &buffer.elements[wIdx];
  ele->data = data;
  ele->len = len;
  ele->write_to = location;

  long dirty_idx = buffer.and_seed & (long) location;
  int r = __sync_fetch_and_add(&(buffer.dirties[dirty_idx]), 1);

  //printf("location = 0x%llx, dirty_idx = %lu, val = %d\n", location, dirty_idx, buffer.dirties[dirty_idx]);

}

void *my_read(void *location) {
  int dirty_idx = buffer.and_seed & (long) location;
  if(buffer.dirties[dirty_idx]) {
    //value is in the buffer
    //Wait for the data to transfer
    my_xfer();
  }

  return (void *) location;
}

void my_xfer() {
  int i, r, dirty_idx;
  write_t *to_write;

  if(transferring) {
    //Another thread is already transferring, just wait for it to finish and return
    goto wait_for_finish;
  }

  r = __sync_bool_compare_and_swap(&transferring, 0, 1);

  if(!r) {
    //Another thread is already transferring, just wait for it to finish and return
    goto wait_for_finish;
  }

  while(buffer.read_idx != buffer.write_idx) {
    i = buffer.read_idx;

    to_write = &(buffer.elements[i]);
    memcpy(to_write->write_to, to_write->data, to_write->len);

    //Cleanup
    dirty_idx = buffer.and_seed & (long) to_write->write_to;
    r = __sync_fetch_and_sub(&(buffer.dirties[dirty_idx]), 1);

    __sync_fetch_and_add(&(buffer.read_idx), 1);
  }

  __sync_bool_compare_and_swap(&transferring, 1, 0);
  return;

wait_for_finish:
  while(transferring);
  return;
}


static rt_mem_t glob_rt_mem = {
  .write = my_write,
  .read = my_read,
  .do_transfer = my_xfer
};

rt_mem_t get_rt_mem() {
  if(!is_initialized) {
    internal_init();
  }
  
  return glob_rt_mem;
}

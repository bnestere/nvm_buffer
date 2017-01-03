#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>

#include "mmp_user.h"

void my_xfer(void);

static int is_initialized = 0, initializing = 0, transferring = 0;
static buffer_t buffer;

/*
 * Used to get the index in the buffer.dirties from the intended nvm address
 */
static inline long hash_addr(long addr) {
  return buffer.and_seed & (addr);
}

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
  int wIdx, r;
  long dirty_idx;

  wIdx = __sync_fetch_and_add(&(buffer.write_idx), 1) & buffer.and_seed;
  //Circular buffer
  if(buffer.write_idx >= WRITE_BUFFER_SIZE) {
    __sync_fetch_and_and(&(buffer.write_idx), buffer.and_seed);
  }

  write_t *ele = &buffer.elements[wIdx];
  ele->data = data;
  ele->len = len;
  ele->write_to = location;
  ele->direct_val = 0;

  dirty_idx = hash_addr((long) location);
  r = __sync_fetch_and_add(&(buffer.dirties[dirty_idx]), 1);
  while(!r) {
    r = __sync_fetch_and_add(&(buffer.dirties[dirty_idx]), 1);
  }
}

/*
 * data doesn't actually point to a value. Instead, the pointer of data is the actual value and continues for len.
 */
void my_write_literal(void *data, int len, void *location) {
  int wIdx, r, *dirty_holder;
  long dirty_idx;

  wIdx = __sync_fetch_and_add(&(buffer.write_idx), 1) & buffer.and_seed;

  //Circular buffer
  if(buffer.write_idx >= WRITE_BUFFER_SIZE) {
    __sync_fetch_and_and(&(buffer.write_idx), buffer.and_seed);
  }

  write_t *ele = &buffer.elements[wIdx];
  ele->len = len;
  ele->write_to = location;
  ele->direct_val = 1;
  memcpy(&(ele->data), &data, len); // Treat the void pointer as a literal value

  dirty_idx = hash_addr((long) location);
  dirty_holder = &(buffer.dirties[dirty_idx]);
  r = __sync_fetch_and_add(dirty_holder, 1);
  while(!r) {
    r = __sync_fetch_and_add(dirty_holder, 1);
  }
}

void *my_read(void *location) {
  int dirty_idx = hash_addr((long) location);
  if(buffer.dirties[dirty_idx]) {
    //value is in the buffer
    //Wait for the data to transfer
    //printf("Doing xfer from read\n");
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
    if(to_write->direct_val) {
      memcpy(to_write->write_to, &(to_write->data), to_write->len);
    } else {
      memcpy(to_write->write_to, to_write->data, to_write->len);
    }

    //Cleanup
    dirty_idx = hash_addr((long) to_write->write_to);
    r = __sync_fetch_and_sub(&(buffer.dirties[dirty_idx]), 1);

    // no need for atomic as one thread will do the full transfer at a time
    buffer.read_idx = (buffer.read_idx + 1) & buffer.and_seed;
  }

  // signal completion of the transferring
  __sync_bool_compare_and_swap(&transferring, 1, 0);
  
  return;

wait_for_finish:
  while(transferring);
  return;
}


static rt_mem_t glob_rt_mem = {
  .write = my_write,
  .write_literal = my_write_literal,
  .read = my_read,
  .do_transfer = my_xfer
};

rt_mem_t *get_rt_mem() {
  if(!is_initialized) {
    internal_init();
  }

  return &glob_rt_mem;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

#include "mmp_user.h"

void my_xfer(void);

static int is_initialized = 0, initializing = 0, transferring = 0;
static buffer_t buffer;

struct timespec tim, tim2;

long hash_addr(long addr) {
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

			tim.tv_sec = 0;
			tim.tv_nsec = 10;

      is_initialized = 1;
    }
  }
}

void my_write(void *data, int len, void *location) {
  int wIdx, r;
  long dirty_idx;

  wIdx = __sync_fetch_and_add(&(buffer.write_idx), 1) & buffer.and_seed;

  write_t *ele = &buffer.elements[wIdx];
  ele->data = data;
  ele->write_to = location;
  ele->direct_val = 0;

  // For reads to know if this value hasn't been moved to NVRAM yet
  dirty_idx = hash_addr((long) location);
  r = __sync_fetch_and_add(&(buffer.dirties[dirty_idx]), 1);
  while(!r) {
    r = __sync_fetch_and_add(&(buffer.dirties[dirty_idx]), 1);
  }

	// Len is set last because it is used to determine if the data is ready to be moved from buffer to nvm
  ele->len = len;
}

/*
 * data doesn't actually point to a value. Instead, the pointer of data is the actual value and continues for len.
 */
void my_write_literal(void *data, int len, void *location) {
  int wIdx, r;
  long dirty_idx;

  wIdx = __sync_fetch_and_add(&(buffer.write_idx), 1) & buffer.and_seed;
  //wIdx = __sync_fetch_and_add(&(buffer.write_idx), 1);
  //if(wIdx >= WRITE_BUFFER_SIZE) {
  //  wIdx = wIdx & buffer.and_seed;
 // }

  write_t *ele = &buffer.elements[wIdx];
  ele->write_to = location;
  ele->direct_val = 1;
  memcpy(&(ele->data), &data, len); // Treat the void pointer as a literal value

  // For reads to know if this value hasn't been moved to NVRAM yet
  dirty_idx = hash_addr((long) location);
  r = __sync_fetch_and_add(&(buffer.dirties[dirty_idx]), 1);
  while(!r) {
    r = __sync_fetch_and_add(&(buffer.dirties[dirty_idx]), 1);
  }

	// Len is set last because it is used to determine if the data is ready to be moved from buffer to nvm
  ele->len = len;
}

void *my_read(void *location) {
  int dirty_idx = hash_addr((long) location);
  if(buffer.dirties[dirty_idx]) {
    //value is in the buffer
    //Wait for the data to transfer
    my_xfer();
  }
  
  return (void *) location;
}

void my_check_self() {

	//  If the buffer indexes have surpassed the buffer size, do a quick fix to put it back
	if(buffer.write_idx >= WRITE_BUFFER_SIZE) {
		__sync_fetch_and_and(&(buffer.write_idx), buffer.and_seed);
	}
}

inline void inline_check_self() {

	//  If the buffer indexes have surpassed the buffer size, do a quick fix to put it back
	if(buffer.write_idx >= WRITE_BUFFER_SIZE) {
		__sync_fetch_and_and(&(buffer.write_idx), buffer.and_seed);
	}
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


  while(buffer.read_idx != (buffer.write_idx & buffer.and_seed)) {
    i = buffer.read_idx;

    to_write = &(buffer.elements[i]);

    // Len is written to last and used here as a "ready" flag
		while(to_write->len == 0) {
      //if(buffer.read_idx == buffer.write_idx & buffer.and_seed) {
        //goto xfer_quit;
      //}
			//	Write index has been moved up, but the data isn't ready yet. Sleep a tad to give the other thread time
			nanosleep(&tim, &tim2); // 10 nanosecs?
		}

    if(to_write->direct_val == 1) {
			//	Treat the value stored in the pointer as literal
      memcpy(to_write->write_to, &(to_write->data), to_write->len);
    } else {
			// Not literal, do a copy like the data is an actual pointer
      memcpy(to_write->write_to, to_write->data, to_write->len);
    }

		//	Cleanup
		to_write->direct_val = 0; 
		to_write->len = 0;
    dirty_idx = hash_addr((long) to_write->write_to);
    r = __sync_fetch_and_sub(&(buffer.dirties[dirty_idx]), 1);

    buffer.read_idx = buffer.read_idx + 1;
    if(buffer.read_idx >= WRITE_BUFFER_SIZE) {
      buffer.read_idx = buffer.read_idx & buffer.and_seed; 
    }
  }

xfer_quit:
  // no need for atomic here because there is only 1 thread doing this at a time
  transferring = 0;
  //__sync_bool_compare_and_swap(&transferring, 1, 0);
  
  return;

wait_for_finish:
  while(transferring) {
    nanosleep(&tim, &tim2);
  }
  return;
}

static rt_mem_t glob_rt_mem = {
  .write = my_write,
  .write_literal = my_write_literal,
  .read = my_read,
  .do_transfer = my_xfer,
	.check_self = my_check_self
};

rt_mem_t *get_rt_mem() {
  if(!is_initialized) {
    internal_init();
  }

  return &glob_rt_mem;
}

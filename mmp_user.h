#ifndef _MMP_USER_
#define _MMP_USER_

#include <stdio.h>
#include <stdlib.h>

// Number of elements to hold in the write buffer
#define WRITE_BUFFER_SIZE 131072

typedef struct rt_mem_t rt_mem_t;
typedef struct buffer_t buffer_t;
typedef struct write_t write_t;

/*
 * Defines the functions available to the user for interaction with the buffer 
 */
struct rt_mem_t {
  void (*write)(void *data, int len, void *location);
  void (*write_literal)(void *d_data, int len, void *location); // The d_data "pointer" is actually the data, don't dereference or else!
  void *(*read)(void *location);
  void (*do_transfer)(void);
};

/*
 * Pointer type for the dram buffer, points to an area in dram to move into nvm
 */
struct write_t {
  void *data;
  void *write_to;
  int len;
  int direct_val;
};

// buffer type for holding the elements in dram to move to nvm
struct buffer_t {
  write_t elements[WRITE_BUFFER_SIZE];
  int dirties[WRITE_BUFFER_SIZE];
  int read_idx;
  int write_idx;
  long and_seed;
};


//Functions
rt_mem_t *get_rt_mem(void);

#endif

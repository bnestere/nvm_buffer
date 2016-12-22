#ifndef _MMP_USER_
#define _MMP_USER_

#include <stdio.h>
#include <stdlib.h>

#define WRITE_BUFFER_SIZE 8192

typedef struct rt_mem_t rt_mem_t;
typedef struct buffer_t buffer_t;
typedef struct write_t write_t;

struct rt_mem_t {
  void (*write)(void *data, int len, void *location);
  void *(*read)(void *location);
  void (*do_transfer)(void);
};

struct write_t {
  void *data;
  int len;
  void *write_to;
};

struct buffer_t {
  write_t elements[WRITE_BUFFER_SIZE];
  int dirties[WRITE_BUFFER_SIZE];
  int read_idx;
  int write_idx;
  long and_seed;
};


//Functions
rt_mem_t get_rt_mem(void);

#endif

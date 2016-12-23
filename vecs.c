#include <sys/mman.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mmp_user.h"
#include "mmp_init.h"

#define VEC_SZ 65536

int main(int argc, char *argv[]) {
  void *a_p, *b_p, *c_p;
  long *a, *b, *c, a_val, b_val;
  int i, j;
  rt_mem_t *rt_mem = get_mmp_initializer()->initialize();

  a_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  b_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  c_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  a = (long *) a_p;
  b = (long *) b_p;
  c = (long *) c_p;

  //rt_mem->write_literal((void *) ((long) 7), sizeof(long), &a[0]);
  //printf("a[%d] = %lu\n", i, *((long *)rt_mem->read(&a[0])));

  // rt_mem->write_literal((void *) ((long) i), sizeof(long), &a[i]);
  // rt_mem->write_literal((void *) ((long) i), sizeof(long), &a[i]);

  for(j = 0; j < 250; j++) {

    //printf("Pre a and b init\n");
    for(i = 0; i < VEC_SZ; i++) {
      //printf("Writing a for j = %d i = %d\n", j, i);
      rt_mem->write_literal((void *) ((long) i), sizeof(long), &a[i]);
      rt_mem->write_literal((void *) ((long) i + VEC_SZ), sizeof(long), &b[i]);
     // printf("Finished writing\n");
      a_val = *((long *) rt_mem->read(&a[i]));
      b_val = *((long *) rt_mem->read(&b[i]));
      //printf("Finished writing j %d i %d, a = %lu, b = %lu\n", j, i, a_val, b_val);
      //printf("Finished reading\n");
    }

    //printf("Pre c compution\n");
    for(i = 0; i < VEC_SZ; i++) {
      a_val = *((long *) rt_mem->read(&a[i]));
      b_val = *((long *) rt_mem->read(&b[i]));
      rt_mem->write_literal((void *) ((long) a_val + b_val), sizeof(long), &c[i]);
    }

    //printf("Pre write direct\n");
    //rt_mem->write_literal((void *) ((long) 22), sizeof(long), a);
    //printf("Post write direct\n");

    printf("Finished computation for j = %d\n", j);
    //for(i = 0; i < VEC_SZ; i++) {
    //  printf("c[%d] = %lu\n", i, *((long *)rt_mem->read(&c[i])));
    //}
  }

  return 0;

}

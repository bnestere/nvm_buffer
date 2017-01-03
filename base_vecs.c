#include <sys/mman.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define VEC_SZ 65536

int main(int argc, char *argv[]) {
  void *a_p, *b_p, *c_p;
  long *a, *b, *c, a_val, b_val;
  int i, j;

  a_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  b_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  c_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  a = (long *) a_p;
  b = (long *) b_p;
  c = (long *) c_p;

  for(j = 0; j < 250; j++) {
    for(i = 0; i < VEC_SZ; i++) {
      a[i] = i;
      b[i] = i + VEC_SZ;
    }

    for(i = 0; i < VEC_SZ; i++) {
      a_val = a[i];
      b_val = b[i];
      c[i] = a_val + b_val;
    }

    printf("Finished computation for j = %d\n", j);
  }

  return 0;

}

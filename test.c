#include <sys/types.h>
#include <malloc.h>
#include <sys/mman.h>
#include <string.h>

#include "mmp_user.h"
#include "mmp_init.h"

int main(int argc, char *argv[]) {
  
  void *nvm, *dram;
  long *np, *dp, *r;
  int i;
  rt_mem_t *rt_mem = get_mmp_initializer()->initialize();

  nvm = mmap(0, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  dram = mmap(0, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  np = (long *) nvm;
  dp = (long *) dram;

  for(i = 0; i < 10; i += 2) {
    dp[i] = i * 2;
    dp[i+1] = dp[i] +  3;
    //dp[i+1] = ((i) * 2) - 4;
    rt_mem->write((void *) &dp[i], 2 * sizeof(long), (void *) &np[i]);
    printf("Reading np[%d] = %lu\n",i, *((long *) rt_mem->read(&np[i])));
    printf("Reading np[%d] = %lu\n",i, *((long *) rt_mem->read(&np[i])));
  }

  for(i = 0; i < 10; i++) {
    printf("pre xfer NVM val is %lu\n", np[i]);
  }

  sleep(1);

  for(i = 0; i < 10; i++) {
    printf("post xfer NVM val is %lu\n", np[i]);
  }
}

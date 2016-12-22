#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  unsigned long data = 0x7ffe18d4;

  int res = 1023 & data;
  int res2 = data % 1024;

  printf("and res = %lu\nmod res = %lu\n", res, res2);
  
  return 0;
}

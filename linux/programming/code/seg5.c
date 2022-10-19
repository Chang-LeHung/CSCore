
#include <stdio.h>
#include <stdint.h>

uint64_t find_rbp() {
  uint64_t rbp;
  asm(
    "movq %%rbp, %0;"
    :"=m"(rbp)::
  );
  return rbp;
}

int main() {

  uint64_t rbp =  find_rbp();
  printf("rbp = %lx\n", rbp);
  // long* p = 0x7ffd4ea724a0;
  printf("%ld\n", *(long*)rbp);
  *(long*)rbp = 100;
  return 0;
}

#include <stdio.h>
#include <unistd.h> 
#include <signal.h>

void sig(int n) {
  write(STDOUT_FILENO, "a", 1);
}

int main() {
  signal(SIGSEGV, sig);
  int* p; 
  printf("%d\n", *p);
  return 0;
}
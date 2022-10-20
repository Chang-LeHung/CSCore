
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sig(int n) {

  printf("直接在这里退出\n");
  _exit(1);
}

int main() {
  signal(SIGSEGV, sig);
  *(int*) NULL = 0;
  printf("结束\n");
  return 0;
}
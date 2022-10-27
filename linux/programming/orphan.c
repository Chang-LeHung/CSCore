

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
  if(fork()) {
    sleep(1);
    // 父进程退出
    exit(0);
  }
  while(1) {
    printf("pid = %d parent pid = %d\n", getpid(), getppid());
    sleep(1);
  }
  return 0;
}
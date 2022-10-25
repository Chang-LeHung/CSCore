


#include <stdio.h>
#include <unistd.h>

int main() {

  for(int i = 0; i < 10; i++) {
    if(!fork())
      break;
  }
  printf("进程ID = %d 进程组ID = %d\n", getpid(), getpgid(0));
  sleep(100);
  return 0;
}

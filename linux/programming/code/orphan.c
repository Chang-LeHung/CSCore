
#include <stdio.h>
#include <unistd.h>

int main(){

  int pid;
  printf("parent pid = %d\n", getpid());
  for(int i = 0; i < 3; i++) {
    pid = fork();
    if(pid == 0)
      break;
  }
  if(pid == 0) {
    while(1) {
      printf("pid = %d pgid = %d\n", getpid(), getpgid(0));
      sleep(5);
    }
  }
  sleep(10);

  return 0;
}
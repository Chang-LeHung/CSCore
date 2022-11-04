
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void sig(int n) {
  switch (n)
  {
  case SIGCONT:
    printf("receive a SIGCONT signal\n");
    break;
  case SIGHUP:
    printf("receive a SIGHUP signal\n");
  }
}

int main(){
  signal(SIGCONT, sig);
  signal(SIGHUP, sig);
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
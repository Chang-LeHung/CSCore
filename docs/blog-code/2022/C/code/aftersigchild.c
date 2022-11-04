

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

void sig(int n) {
  if(n == SIGCHLD) {
    printf("received a sigchild signal\n");
  }
  int nwait;
  int id = wait(&nwait);
  printf("pid = %d\n", id);
}

int main() {
  signal(SIGCHLD, sig);
  printf("pid = %d\n", getpid());
  if(fork()) {
    while(1);
  }
  printf("pid = %d parent pid = %d\n", getpid(), getppid());
  sleep(10);
  return 0;
}
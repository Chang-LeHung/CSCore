
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

void sig(int no) {
  char out[128];
  switch(no) {
    case SIGINT:
      sprintf(out, "received SIGINT signal\n");
      break;
    case SIGTSTP:
      sprintf(out, "received SIGSTOP signal\n");
      break;
  }
  write(STDOUT_FILENO, out, strlen(out));
}

int main() {
  signal(SIGINT, sig);
  signal(SIGTSTP, sig);
  while(1) {sleep(1);}
  return 0;
}
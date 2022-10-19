
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

void sig(int n) {
  char* str = "signal number = %d\n";
  char* out = malloc(128);
  sprintf(out, str, n);
  write(STDOUT_FILENO, out, strlen(out));
  free(out);
}

int main() {
  signal(SIGINT, sig);
  printf("pid = %d\n", getpid());
  while (1)
  {
    sleep(1);
  }
  
  return 0;
}
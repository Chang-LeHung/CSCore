
#include <sys/time.h>
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
  if(fork() != 0) {
    struct rusage usage;
    int pid = wait4(-1, NULL, NULL, &usage);
    printf("pid = %d memory usage peek = %ldkb\n", pid, usage.ru_maxrss);
  }else {
    execv("./getrusage.out", NULL);
  }
  return 0;
}
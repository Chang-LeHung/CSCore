

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

void sig(int n) {
  int fd = open("test.out", O_RDWR | O_CREAT);
  char* out = malloc(128);
  sprintf(out, "hello world from pid = %d\n", getpid());
  write(fd, out, strlen(out));
  write(1, out, strlen(out));
  fsync(fd);
  close(fd);
}

int main() {
  
  printf("pid = %d pgid = %d\n", getpid(), getpgid(0));
  signal(SIGHUP, sig);
  while (1)
  {
    printf("hello world\n");
    sleep(1);
  }
  
}
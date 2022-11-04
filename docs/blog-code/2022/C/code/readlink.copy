

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>


int main(int argc, char* argv[]) {

  char buf[1024];
  size_t s = readlink(argv[1], buf, 1024);
  buf[s] = '\0';
  printf("size = %ld\n", s);
  sprintf(buf, "a = %d\n\0", 1);
  printf(buf);
  return 0;
}


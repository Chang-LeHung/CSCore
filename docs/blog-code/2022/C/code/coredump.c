

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {

  if(fork()) {
    int s;
    wait(&s);
    if(WCOREDUMP(s)) {
      printf("core dump true\n");
    }
  }else{
    int a = *(int*)NULL;
  }
}

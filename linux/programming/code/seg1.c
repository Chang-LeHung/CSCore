
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <setjmp.h>

jmp_buf env;

void sig(int no) {
  write(1, "a", 1);
  // longjmp(env, 1);
}

char* str= "hello world";

int main() {
  signal(SIGSEGV, sig);
  printf("%s\n", str);
  int* p;
  *p = 1;
  return 0;
}
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

int main()
{
  signal(SIGSEGV, sig);
  //  int n = 2;
  //  scanf(" ",n);
  // if(setjmp(env)) {
  //   int*p;
  //   *p = 1;
  // }else {
  //   printf("finished\n");
  // }
  int*p;
  *p = 1;
   return 0;
}

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>

jmp_buf env;

void sig(int n) {
  printf("准备回到主函数\n");
  longjmp(env, 1);
}

int main() {
  signal(SIGSEGV, sig);
  if(!setjmp(env)) {
    printf("产生段错误\n");
    *(int*) NULL = 0;
  }else {
    printf("回到了主函数\n");
  }
  return 0;
}
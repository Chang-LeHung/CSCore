
#include <sys/time.h>
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
  if(fork() != 0) {
    struct rusage usage; // 定义一个统计资源的结构体
    int pid = wait4(-1, NULL, 0, &usage); // 将这个结构体的地址传入 好让内核能讲对应的信息存在指针所指向的地方
    // 打印内存使用的峰值
    printf("pid = %d memory usage peek = %ldkb\n", pid, usage.ru_maxrss);
  }else {
    execv("./getrusage.out", argv);
  }
  return 0;
}
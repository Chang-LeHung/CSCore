# Pthread 并发编程（二）——自底向上深入理解线程

## 前言
在本篇文章当中主要给大家介绍线程最基本的组成元素，以及在 pthread 当中给我们提供的一些线程的基本机制。希望能够帮助大家深入理解线程。

## 线程的基本元素

首先我们需要了解一些我们在使用线程的时候的常用的基本操作，如果不是很了解没有关系我们在后续的文章当中会仔细谈论这些问题。
- 线程的常见的基本操作：
  - 线程的创建。
  - 线程的终止。
  - 线程之间的同步。
  - 线程的调度。
  - 线程当中的数据管理。
  - 线程与进程之间的交互。
- 在 linux 当中所有的线程和进程共享一个地址空间。
- 进程与线程之间共享一些内核数据结构：
  - 打开的文件描述符。
  - 当前工作目录。
  - 用户 id 和用户组 id 。
  - 全局数据段的数据。
  - 进程的代码。
  - 信号（signals）和信号处理函数（signal handlers）。

- 线程独有的：
  - 线程的 ID 。
  - 寄存器线程和栈空间。
  - 线程的栈当中的局部变量和返回地址。
  - 信号掩码。
  - 线程自己的优先级。
  - errno。

在所有的 pthread 的接口当中，只有当函数的返回值是 0 的时候表示调用成功。

## 线程的属性

### 线程等待
在 pthread 的实现当中，每个线程都两个特性：joinable 和 detached，当我们启动一个线程的时候 (pthread_create) 线程的默认属性是 joinable，所谓 joinable 是表示线程是可以使用 pthread_join 进行同步的。

当一个线程调用 pthread_join(T, ret)，当这个函数返回的时候就表示线程 T 已经终止了，执行完成。那么就可以释放与线程 T 的相关的系统资源。

如果一个线程的状态是 detached 状态的话，当线程结束的时候与这个线程相关的资源会被自动释放掉，将资源归还给系统，也就不需要其他的线程调用 pthread_join 来释放线程的资源。

pthread_join 函数签名如下：
```
int pthread_join(pthread_t thread, void **retval);
```

- thread 表示等待的线程。
- retval 如果 retval 不等于 NULL 则在 pthread_join 函数内部会将线程 thead 的退出状态拷贝到 retval 指向的地址。如果线程被取消了，那么 PTHREAD_CANCELED 将会被放在 retval 指向的地址。
- 函数的返回值
  - EDEADLK 表示检测到死锁了，比入两个线程都调用 pthread_join 函数等待对方执行完成。
  - EINVAL 线程不是一个 joinable 的线程，一种常见的情况就是 pthread_join 一个 detached 线程。
  - EINVAL 当调用 pthrea_join 等待的线程正在被别的线程调用 pthread_join 等待。
  - ESRCH 如果参数 thread 是一个无效的线程，比如没有使用 pthread_create 进行创建。
  - 0 表示函数调用成功。


在下面的程序当中我们使用 pthread_join 函数去等待一个 detached 线程：
```c
#include <stdio.h>
#include <error.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

pthread_t t1, t2;

void* thread_1(void* arg) {

  int ret = pthread_detach(pthread_self());
  sleep(2);
  if(ret != 0)
    perror("");
  return NULL;
}


int main() {

  pthread_create(&t1, NULL, thread_1, NULL);
  sleep(1);
  int ret = pthread_join(t1, NULL);
  if(ret == ESRCH)
    printf("No thread with the ID thread could be found.\n");
  else if(ret == EINVAL) {
    printf("thread is not a joinable thread or Another thread is already waiting to join with this thread\n");
  }
  return 0;
}

```
上面的程序的输出结果如下所示：

```
$ ./join.out
thread is not a joinable thread or Another thread is already waiting to join with this thread
```
在上面的程序当中我们在一个 detached 状态的线程上使用 pthread_join 函数，因此函数的返回值是 EINVAL 表示线程不是一个 joinable 的线程。

在上面的程序当中 pthread_self() 返回当前正在执行的线程，返回的数据类型是 pthread_t ，函数 pthread_detach(thread) 的主要作用是将传入的线程 thread 的状态变成 detached 状态。

我们再来看一个错误的例子，我们在一个无效的线程上调用 pthread_join 函数

```c


#include <stdio.h>
#include <error.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

pthread_t t1, t2;

void* thread_1(void* arg) {

  int ret = pthread_detach(pthread_self());
  sleep(2);
  if(ret != 0)
    perror("");
  return NULL;
}


int main() {

  pthread_create(&t1, NULL, thread_1, NULL);
  sleep(1);
  int ret = pthread_join(t2, NULL);
  if(ret == ESRCH)
    printf("No thread with the ID thread could be found.\n");
  else if(ret == EINVAL) {
    printf("thread is not a joinable thread or Another thread is already waiting to join with this thread\n");
  }
  return 0;
}

```
上面的程序的输出结果如下：

```
$./oin01.out
No thread with the ID thread could be found.
```
在上面的程序当中我们并没有使用 t2 创建一个线程但是在主线程执行的代码当中，我们使用 pthread_join 去等待他，因此函数的返回值是一个 EINVAL 。

我们再来看一个使用 retval 例子：
```c
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>

void* func(void* arg)
{
  pthread_exit((void*)100);
  return NULL;
}

int main() {
  pthread_t t;
  pthread_create(&t, NULL, func, NULL);
  
  void* ret;
  pthread_join(t, &ret);
  printf("ret = %ld\n", (u_int64_t)(ret));
  return 0;
}
```
上面的程序的输出结果如下所示：
```
$./understandthread/join03.out
ret = 100
```
在上面的程序当中我们使用一个参数 ret 去获取线程的退出码，从上面的结果我们可以知道，我们得到了正确的结果。

如果我们没有在线程执行的函数当中使用 pthread_exit 函数当中明确的指出线程的退出码，线程的退出码就是函数的返回值。比如下面的的程序：
```c
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>

void* func(void* arg)
{
  return (void*)100;
}

int main() {
  pthread_t t;
  pthread_create(&t, NULL, func, NULL);
  
  void* ret;
  pthread_join(t, &ret);
  printf("ret = %ld\n", (u_int64_t)(ret));
  return 0;
}
```
上面的程序的输出结果也是 100 ，这与我们期待的结果是一致的。

### 获取线程的栈帧

在多线程的程序当中，每个线程拥有自己的栈帧和PC寄存器（执行的代码的位置，在 x86_86 里面就是 rip 寄存器的值）。在下面的程序当中我们可以得到程序在执行时候的三个寄存器 rsp, rbp, rip 的值，我们可以看到，两个线程执行时候的输出是不一致的，这个也从侧面反映出来线程是拥有自己的栈帧和PC值的。

```c
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>

u_int64_t rsp;
u_int64_t rbp;
u_int64_t rip;

void find_rip() {
  asm volatile(
    "movq 8(%%rbp), %0;"
    :"=r"(rip)::
  );
}

void* func(void* arg) {
  printf("In func\n");
  asm volatile(             \
    "movq %%rsp, %0;"       \
    "movq %%rbp, %1;"       \
    :"=m"(rsp), "=m"(rbp):: \
  );
  find_rip();
  printf("stack frame: rsp = %p rbp = %p rip = %p\n", (void*)rsp, (void*)rbp, (void*) rip);
  return NULL;
}

int main() {
  printf("================\n");
  printf("In main\n");
  asm volatile(             \
    "movq %%rsp, %0;"       \
    "movq %%rbp, %1;"       \
    :"=m"(rsp), "=m"(rbp):: \
  );
  find_rip();
  printf("stack frame: rsp = %p rbp = %p rip = %p\n", (void*)rsp, (void*)rbp, (void*) rip);
  printf("================\n");
  pthread_t t;
  pthread_create(&t, NULL, func, NULL);
  pthread_join(t, NULL);
  return 0;
}

```

上面的程序的输出结果如下所示：

```
================
In main
stack frame: rsp = 0x7ffc47096d50 rbp = 0x7ffc47096d80 rip = 0x4006c6
================
In func
stack frame: rsp = 0x7f0a60d43ee0 rbp = 0x7f0a60d43ef0 rip = 0x400634
```

从上面的结果来看主线程和线程 t 执行的是不同的函数，而且两个函数的栈帧差距还是很大的，我们计算一下 0x7ffc47096d80 - 0x7f0a60d43ef0 = 1038949363344 = 968G 的内存，因此很明显这两个线程使用的是不同的栈帧。

### 线程的线程号

在 pthread 当中的一个线程对应一个内核的线程，内核和 pthread 都给线程维护了一个线程的 id 号，我们可以使用 gettid 获取操作系统给我们维护的线程号，使用函数 pthread_self 得到 pthread 线程库给我们维护的线程号！

```c

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

void* func(void* arg) {
  printf("pthread id = %ld tid = %d\n", pthread_self(), (int)gettid());
  return NULL;
}

int main() {
  pthread_t t;
  pthread_create(&t, NULL, func, NULL);
  pthread_join(t, NULL);
  return 0;
}
```

上面的程序的输出结果如下：

```
pthread id = 140063790135040 tid = 161643
```






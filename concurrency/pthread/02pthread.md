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

## 通过例子理解线程的基本属性

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
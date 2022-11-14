# Pthread 并发编程（二）——自底向上深入理解线程

## 前言
在本篇文章当中主要给大家介绍线程最基本的组成元素，以及在 pthread 当中给我们提供的一些线程的基本机制，因为很多语言的线程机制就是建立在 pthread 线程之上的，比如说 Python 和 Java，深入理解 pthread 的线程实现机制，可以极大的提升我们对于语言线程的认识。希望能够帮助大家深入理解线程。

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



## 线程等待
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

## 获取线程的栈帧和PC值

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

## 线程的线程号

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

## 线程与信号

在 pthread 库当中主要给我们提供了一些函数用于信号处理，我们在 pthread 库当中可以通过函数 pthread_kill 给其他的进程发送信号。

```
 1) SIGHUP	 2) SIGINT	 3) SIGQUIT	 4) SIGILL	 5) SIGTRAP
 6) SIGABRT	 7) SIGBUS	 8) SIGFPE	 9) SIGKILL	10) SIGUSR1
11) SIGSEGV	12) SIGUSR2	13) SIGPIPE	14) SIGALRM	15) SIGTERM
16) SIGSTKFLT	17) SIGCHLD	18) SIGCONT	19) SIGSTOP	20) SIGTSTP
21) SIGTTIN	22) SIGTTOU	23) SIGURG	24) SIGXCPU	25) SIGXFSZ
26) SIGVTALRM	27) SIGPROF	28) SIGWINCH	29) SIGIO	30) SIGPWR
31) SIGSYS	34) SIGRTMIN	35) SIGRTMIN+1	36) SIGRTMIN+2	37) SIGRTMIN+3
38) SIGRTMIN+4	39) SIGRTMIN+5	40) SIGRTMIN+6	41) SIGRTMIN+7	42) SIGRTMIN+8
43) SIGRTMIN+9	44) SIGRTMIN+10	45) SIGRTMIN+11	46) SIGRTMIN+12	47) SIGRTMIN+13
48) SIGRTMIN+14	49) SIGRTMIN+15	50) SIGRTMAX-14	51) SIGRTMAX-13	52) SIGRTMAX-12
53) SIGRTMAX-11	54) SIGRTMAX-10	55) SIGRTMAX-9	56) SIGRTMAX-8	57) SIGRTMAX-7
58) SIGRTMAX-6	59) SIGRTMAX-5	60) SIGRTMAX-4	61) SIGRTMAX-3	62) SIGRTMAX-2
63) SIGRTMAX-1	64) SIGRTMAX
```

我们可以在一个线程当中响应其他线程发送过来的信号，并且响应信号处理函数，在使用具体的例子深入了解线程的信号机制之前，首先我们需要了解到的是在 pthread 多线程的程序当中所有线程是共享信号处理函数的，如果在一个线程当中修改了信号处理函数，这个结果是会影响其他线程的。

```c

#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

void sig(int signo) {
  char s[1024];
  sprintf(s, "signo = %d tid = %d pthread tid = %ld\n", signo, gettid(), pthread_self());
  write(STDOUT_FILENO, s, strlen(s));
}

void* func(void* arg) {
  printf("pthread tid = %ld\n", pthread_self());
  for(;;);
  return NULL;
}

int main() {
  signal(SIGHUP, sig);
  signal(SIGTERM, sig);
  signal(SIGSEGV, sig);
  pthread_t t;
  pthread_create(&t, NULL, func, NULL);

  sleep(1);
  pthread_kill(t, SIGHUP);
  sleep(1);
  return 0;
}
```

上面的程序的输出结果如下所示：

```
pthread tid = 140571386894080
signo = 1 tid = 7785 pthread tid = 140571386894080
```

在上面的程序当中，我们首先在主函数里面重新定义了几个信号的处理函数，将 SIGHUP、SIGTERM 和 SIGSEGV 信号的处理函数全部声明为函数 sig ，进程当中的线程接受到这个信号的时候就会调用对应的处理函数，在上面的程序当中主线程会给线程 t 发送一个 SIGHUP 信号，根据前面信号和数据对应关系我们可以知道 SIGHUP 对应的信号的数字等于 1 ，我们在信号处理函数当中确实得到了这个信号。

除此之外我们还可以设置线程自己的信号掩码，在前文当中我们已经提到了，每个线程都拥有线程自己的掩码，因此在下面的程序当中只有线程 2 响应了主线程发送的 SIGTERM 信号。

```c

#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

void sig(int signo) {
  char s[1024];
  sprintf(s, "signo = %d tid = %d pthread tid = %ld\n", signo, gettid(), pthread_self());
  write(STDOUT_FILENO, s, strlen(s));
}

void* func(void* arg) {
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &set, NULL);
  // 上面的代码的功能是阻塞 SIGTERM 这个信号 当这个信号传输过来的时候不会立即执行信号处理函数
  // 而是会等到将这个信号变成非阻塞的时候才会响应
  printf("func : pthread tid = %ld\n", pthread_self());
  for(;;);
  return NULL;
}

void* func02(void* arg) {
  printf("func02 : pthread tid = %ld\n", pthread_self());
  for(;;);
  return NULL;
}

int main() {
  signal(SIGTERM, sig);
  pthread_t t1;
  pthread_create(&t1, NULL, func, NULL);
  sleep(1);
  pthread_t t2;
  pthread_create(&t2, NULL, func02, NULL);
  sleep(1);
  pthread_kill(t1, SIGTERM);
  pthread_kill(t2, SIGTERM);
  sleep(2);
  return 0;
}
```

在上面的程序当中我们创建了两个线程并且定义了 SIGTERM 的信号处理函数，在线程 1 执行的函数当中修改了自己阻塞的信号集，将 SIGTERM 变成了一种阻塞信号，也就是说当线程接受到 SIGTERM 的信号的时候不会立即调用 SIGTERM 的信号处理函数，只有将这个信号变成非阻塞的时候才能够响应这个信号，执行对应的信号处理函数，但是线程 t2 并没有阻塞信号 SIGTERM ，因此线程 t2 会执行对应的信号处理函数，上面的程序的输出结果如下所示：

```
func : pthread tid = 139887896323840
func02 : pthread tid = 139887887931136
signo = 15 tid = 10652 pthread tid = 139887887931136
```

根据上面程序的输出结果我们可以知道线程 t2 确实调用了信号处理函数（根据 pthread tid ）可以判断，而线程 t1 没有执行信号处理函数。

在上文当中我们还提到了在一个进程当中，所有的线程共享同一套信号处理函数，如果在一个线程里面重新定义了一个信号的处理函数，那么他将会影响其他的线程，比如下面的程序：

```c

#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

void sig(int signo) {
  char s[1024];
  sprintf(s, "signo = %d tid = %d pthread tid = %ld\n", signo, gettid(), pthread_self());
  write(STDOUT_FILENO, s, strlen(s));
}

void sig2(int signo) {
  char* s = "thread-defined\n";
  write(STDOUT_FILENO, s, strlen(s));
}

void* func(void* arg) {
  signal(SIGSEGV, sig2);
  printf("pthread tid = %ld\n", pthread_self());
  for(;;);
  return NULL;
}

void* func02(void* arg) {
  printf("pthread tid = %ld\n", pthread_self());
  for(;;);
  return NULL;
}

int main() {
  signal(SIGSEGV, sig);
  pthread_t t;
  pthread_create(&t, NULL, func, NULL);
  sleep(1);
  pthread_t t2;
  pthread_create(&t2, NULL, func02, NULL);
  sleep(1);
  pthread_kill(t2, SIGSEGV);
  sleep(2);
  return 0;
}
```

上面的程序的输出结果如下所示：

```
pthread tid = 140581246330624
pthread tid = 140581237937920
thread-defined
```

从上面程序输出的结果我们可以看到线程 t2 执行的信号处理函数是 sig2 而这个信号处理函数是在线程 t1 执行的函数 func 当中进行修改的，可以看到线程 t1 修改的结果确实得到了响应，从这一点也可以看出，如果一个线程修改信号处理函数是会影响到其他的线程的。

## 总结

在本篇文章当中主要介绍了一些基础了线程自己的特性，并且使用一些例子去验证了这些特性，帮助我们从根本上去理解线程，其实线程涉及的东西实在太多了，在本篇文章里面只是列举其中的部分例子进行使用说明，在后续的文章当中我们会继续深入的去谈这些机制，比如线程的调度，线程的取消，线程之间的同步等等。

更多精彩内容合集可访问项目：<https://github.com/Chang-LeHung/CSCore>

关注公众号：一无是处的研究僧，了解更多计算机（Java、Python、计算机系统基础、算法与数据结构）知识。

![](https://img2022.cnblogs.com/blog/2519003/202207/2519003-20220703200459566-1837431658.jpg)


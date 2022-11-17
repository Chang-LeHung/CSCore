# Pthread 并发编程（三）——深入理解线程取消机制

## 基本介绍

线程取消机制是 pthread 给我们提供的一种用于取消线程执行的一种机制，这种机制是在线程内部实现的，仅仅能够在共享内存的多线程程序当中使用。

## 基本使用

```c


#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>


void* task(void* arg) {
  usleep(10);
  printf("step1\n");
  printf("step2\n");
  printf("step3\n");
  return NULL;
}

int main() {

  void* res;
  pthread_t t1;
  pthread_create(&t1, NULL, task, NULL);
  int s = pthread_cancel(t1);
  if(s != 0) // s == 0 mean call successfully
    fprintf(stderr, "cancel failed\n");
  pthread_join(t1, &res);
  assert(res == PTHREAD_CANCELED);
  return 0;
}
```

上面的程序的输出结果如下：

```
step1
```

在上面的程序当中，我们使用一个线程去执行函数 task，然后主线程会执行函数 `pthread_cancel` 去取消线程的执行，从上面程序的输出结果我们可以知道，执行函数 task 的线程并没有执行完成，只打印出了 step1 ，这说明线程被取消执行了。

## 深入分析线程取消机制

在上文的一个例子当中我们简单的使用了一下线程取消机制，在本小节当中将深入分析线程的取消机制。在线程取消机制当中，如果一个线程被正常取消执行了，其他线程使用 pthread_join 去获取线程的退出状态的话，线程的退出状态为 PTHREAD_CANCELED 。比如在上面的例子当中，主线程取消了线程 t1 的执行，然后使用 pthread_join 函数等待线程执行完成，并且使用参数 res 去获取线程的退出状态，在上面的代码当中我们使用 assert 语句去判断 res 的结果是否等于 PTHREAD_CANCELED ，从程序执行的结果来看，assert 通过了，因此线程的退出状态验证正确。

我们来看一下 pthread_cancel 函数的签名：

```
int pthread_cancel(pthread_t thread);
```

函数的返回值：

- 0 	表示函数 pthread_cancel 执行成功。
- ESRCH   表示在系统当中没有 thread 这个线程。这个宏包含在头文件 <errno.h> 当中。

我们现在使用一个例子去测试一下返回值 ESRCH ：

```c
#include <stdio.h>
#include <pthread.h>
#include <errno.h>

int main() {

  pthread_t t;
  int s = pthread_cancel(t);
  if(s == ESRCH)
    printf("No thread with the ID thread could be found.\n");
  return 0;
}
```

上述程序的会执行打印字符串的语句，因为我们并没有使用变量 t 去创建一个线程，因此线程没有创建，返回对应的错误。

### pthread_cancel 的执行

pthread_cancel 函数会发送一个取消请求到指定的线程，线程是否响应这个线程取消请求取决于线程的取消状态和取消类型。

两种线程的取消状态：

- PTHREAD_CANCEL_ENABLE 线程默认是开启响应取消请求，这个状态是表示会响应其他线程发送过来的取消请求，但是具体是如何响应，取决于线程的取消类型。

- PTHREAD_CANCEL_DISABLE 当开启这个选项的时候，调用这个方法的线程就不会响应其他线程发送过来的取消请求。

两种取消类型：

- PTHREAD_CANCEL_DEFERRED 如果线程的取消类型是这个，那么线程将会在下一次调用一个取消点的函数时候取消执行，取消点函数有 read, write, pread, pwrite, sleep 等函数，更多的可以网上搜索。
- PTHREAD_CANCEL_ASYNCHRONOUS 这个取消类型线程就会立即响应发送过来的请求，本质上在 pthread 实现的代码当中是会给线程发送一个信号，然后接受取消请求的线程在信号处理函数当中进行退出。

### 让线程取消机制无效

```c

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* func(void* arg)
{
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  sleep(1);
  return NULL;
}

int main() {
  pthread_t t;
  pthread_create(&t, NULL, func, NULL);
  pthread_cancel(t);
  void* res;
  pthread_join(t, &res);
  if(res == PTHREAD_CANCELED)
  {
    printf("thread was canceled\n");
  }
  return 0;
}
```

上面的程序不会执行这句话 `printf("thread was canceled\n");` 因为在线程当中设置了线程的状态为不开启线程取消机制，因此主线程发送的取消请求无效。

在上面的代码当中使用的函数 pthread_setcancelstate 的函数签名如下：

```c
int pthread_setcancelstate(int state, int *oldstate)
```

其中第二个参数我们可以传入一个 int 类型的指针，然后会将旧的状态存储到这个值当中。

### 取消点测试

在前文当中我们谈到了，线程的取消机制是默认开启的，但是当一个线程发送取消请求之后，只有等到下一个是取消点的函数的时候，线程才会真正退出取消执行。

```c

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* func(void* arg)
{
  // 默认是 enable  线程的取消机制是开启的
  while(1);
  return NULL;
}

int main() {
  pthread_t t;
  pthread_create(&t, NULL, func, NULL);
  pthread_cancel(t);
  void* res;
  pthread_join(t, &res);
  if(res == PTHREAD_CANCELED)
  {
    printf("thread was canceled\n");
  }
  return 0;
}
```

如果我们去执行上面的代码我们会发现程序会进行循环，不会退出，因为虽然主线程给线程 t 发送了一个取消请求，但是线程 t 一直在进行死循环操作，并没有执行任何一个函数，更不用提是一个取消点函数了。

如果我们修改上面的代码成下面这样，那么线程就会正常执行退出：

```c

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* func(void* arg)
{
  // 默认是 enable  线程的取消机制是开启的
  while(1)
  {
    sleep(1);
  }
  return NULL;
}

int main() {
  pthread_t t;
  pthread_create(&t, NULL, func, NULL);
  pthread_cancel(t);
  void* res;
  pthread_join(t, &res);
  if(res == PTHREAD_CANCELED)
  {
    printf("thread was canceled\n");
  }
  return 0;
}
```

上面的代码唯一修改的地方就是在线程 t 当中的死循环处调用了 sleep 函数，而 sleep 函数是一个取消点函数，因此当主线程给线程 t 发送一个取消请求之后，线程 t 就会在下一次调用 sleep 函数彻底取消执行，退出，并且线程的退出状态为 PTHREAD_CANCELED ，因此主线程会执行代码 `printf("thread was canceled\n");`。


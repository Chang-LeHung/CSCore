# Pthread 并发编程（一）——深入剖析线程基本元素和状态

## 前言

在本篇文章当中讲主要给大家介绍 `pthread` 并发编程当中关于线程的基础概念，并且深入剖析进程的相关属性和设置，以及线程在内存当中的布局形式，帮助大家深刻理解线程。

## 深入理解 pthread_create

### 基础例子介绍

在深入解析 `pthread_create` 之前，我们先用一个简单的例子简单的认识一下 pthread，我们使用 pthread 创建一个线程并且打印 Hello world 字符串。

```c


#include <stdio.h>
#include <pthread.h>

void* func(void* arg) {
  printf("Hello World from tid = %ld\n", pthread_self()); // pthread_self 返回当前调用这个函数的线程的线程 id
  return NULL;
}

int main() {

  pthread_t t; // 定义一个线程
  pthread_create(&t, NULL, func, NULL); // 创建线程并且执行函数 func 

  // wait unit thread t finished
  pthread_join(t, NULL); // 主线程等待线程 t 执行完成然后主线程才继续往下执行

  printf("thread t has finished\n");
  return 0;
}
```

编译上述程序：

```shell
clang helloworld.c -o helloworld.out -lpthread
或者
gcc helloworld.c -o helloworld.out -lpthread
```

在上面的代码当中主线程（可以认为是执行主函数的线程）首先定义一个线程，然后创建线程并且执行函数 func ，当创建完成之后，主线程使用 pthread_join 阻塞自己，直到等待线程 t 执行完成之后主线程才会继续往下执行。

我们现在仔细分析一下 `pthread_create` 的函数签名，并且对他的参数进行详细分析：

```c
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);
```

- 参数 thread 是一个类型为 pthread_t 的指针对象，将这个对象会在 pthread_create 内部会被赋值为存放线程 id 的地址，在后文当中我们将使用一个例子仔细的介绍这个参数的含义。
- 参数 attr 是一个类型为 pthread_attr_t 的指针对象，我们可以在这个对象当中设置线程的各种属性，比如说线程取消的状态和类别，线程使用的栈的大小以及栈的初始位置等等，在后文当中我们将详细介绍这个属性的使用方法，当这个属性为 NULL 的时候，使用默认的属性值。
- 参数 start_routine 是一个返回类型为 void* 参数类型为 void* 的函数指针，指向线程需要执行的函数，线程执行完成这个函数之后线程就会退出。
- 参数 arg ，传递给函数 start_routine 的一个参数，在上一条当中我们提到了 start_routine 有一个参数，是一个 void 类型的指针，这个参数也是一个 void 类型的指针，在后文当中我们使用一个例子说明这个参数的使用方法。

### 深入理解参数 thread

在下面的例子当中我们将使用 pthread_self 得到线程的 id ，并且通过保存线程 id 的地址的变量 t 得到线程的 id ，对两个得到的结果进行比较。

```c


#include <stdio.h>
#include <pthread.h>

void* func(void* arg) {

  printf("线程自己打印线程\tid = %ld\n", pthread_self());

  return NULL;
}

int main() {

  pthread_t t;
  pthread_create(&t, NULL, func, NULL);
  printf("主线程打印线程 t 的线程 id = %ld\n", *(long*)(&t));
  pthread_join(t, NULL);
  return 0;
}
```

上面程序的执行结果如下图所示：

![01](../../images/pthread/03.png)

根据上面程序打印的结果我们可以知道，变量 `pthread_t t` 保存的就是线程 id 的地址， 参数 t 和线程 id 之间的关系如下所示：

![01](../../images/pthread/04.png)

在上面的代码当中我们首先对 t 取地址，然后将其转化为一个 long 类型的指针，然后解引用就可以得到对应地址的值了，也就是线程的ID。

### 深入理解参数 arg

在下面的程序当中我们定义了一个结构体用于保存一些字符出的信息，然后创建一个这个结构体的对象，将这个对象的指针作为参数传递给线程要执行的函数，并且在线程内部打印字符串当中的内容。

```c


#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>


typedef struct info {
  char s[1024]; // 保存字符信息
  int  size;    // 保存字符串的长度
}info_t;

static
void* func(void* arg) {
  info_t* in = (info_t*)arg;
  in->s[in->size] = '\0';
  printf("string in arg = %s\n", in->s);
  return NULL;
}

int main() {

  info_t* in = malloc(sizeof(info_t)); // 申请内存空间
  // 保存 HelloWorld 这个字符串 并且设置字符串的长度
  in->s[0] = 'H';
  in->s[1] = 'e';
  in->s[2] = 'l';
  in->s[3] = 'l';
  in->s[4] = 'o';
  in->s[5] = 'W';
  in->s[6] = 'o';
  in->s[7] = 'r';
  in->s[8] = 'l';
  in->s[9] = 'd';
  in->size = 10;
  pthread_t t;									// 将 in 作为参数传递给函数 func
  pthread_create(&t, NULL, func, (void*)in); 
  pthread_join(t, NULL);
  free(in); // 释放内存空间
  return 0;
}
```

上面程序的执行结果如下所示：

![01](../../images/pthread/05.png)

可以看到函数参数已经做到了正确传递。

### 深入理解参数 attr

在深入介绍参数 attr 前，我们首先需要了解一下程序的内存布局，在64位操作系统当中程序的虚拟内存布局大致如下所示，从下往上依次为：只读数据/代码区、可读可写数据段、堆区、共享库的映射区、程序栈区以及内核内存区域。我们程序执行的区域就是在栈区。

![03](../../images/programming/03.png)

根据上面的虚拟内存布局示意图，我们将其简化一下得到单个线程的执行流和大致的内存布局如下所示（程序执行的时候有他的栈帧以及寄存器现场，图中将寄存器也做出了标识）：

![01](../../images/pthread/01.png)

程序执行的时候当我们进行函数调用的时候函数的栈帧就会从上往下生长，我们现在进行一下测试，看看程序的栈帧最大能够达到多少。

```c


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
int times = 1;

void* func(void* arg) {
  char s[1 << 20]; // 申请 1MB 内存空间（分配在栈空间上）
  printf("times = %d\n", times);
  times++;
  func(NULL);
  return NULL;
}

int main() {

  func(NULL);
  return 0;
}
```

上述程序的执行结果如下图所示：

![01](../../images/pthread/06.png)

从上面的程序我们可以看到在第 8 次申请栈内存的时候遇到了段错误，因此可以判断栈空间大小在 8MB 左右，事实上我们可以查看 linux 操作系统上，栈内存的指定大小：

![01](../../images/pthread/07.png)

事实上在 linux 操作系统当中程序的栈空间的大小默认最大为 8 MB。

现在我们来测试一下，当我们创建一个线程的时候，线程的栈的大小大概是多少：

```c


#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
int times = 1;

void* func(void* arg) {
  printf("times = %d\n", times);
  times++;
  char s[1 << 20]; // 申请 1MB 内存空间（分配在栈空间上）
  func(NULL);
  return NULL;
}

int main() {

  pthread_t t;
  pthread_create(&t, NULL, func, NULL);
  pthread_join(t, NULL);
  return 0;
}
```

上面的程序执行结果如下图所示，可以看到当我们创建一个线程的时候栈的最大的大小也是 8MB。

![01](../../images/pthread/08.png)

#### 设置线程栈空间的大小

现在如果我们有一个需求，需要的栈空间大于 8MB，我们应该怎么办呢？这就是我们所需要谈到的 attr，这个变量是一个 **pthread_attr_t** 对象，这个对象的主要作用就是用于设置线程的各种属性的，其中就包括线程的栈的大小，在下面的程序当中我们将线程的栈空间的大小设置成 24MB，并且使用程序进行测试。

```c

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define MiB * 1 << 20

int times = 0;
void* stack_overflow(void* args) {
  printf("times = %d\n", ++times);
  char s[1 << 20]; // 1 MiB
  stack_overflow(NULL);
  return NULL;
}

int main() {
  pthread_attr_t attr;
  pthread_attr_init(&attr); // 对变量 attr 进行初始化操作
  pthread_attr_setstacksize(&attr, 24 MiB); // 设置栈帧大小为 24 MiB 这里使用了一个小的 trick 大家可以看一下 MiB 的宏定义
  pthread_t t;
  pthread_create(&t, &attr, stack_overflow, NULL);
  pthread_join(t, NULL);
  pthread_attr_destroy(&attr); // 释放线程属性的相关资源
  return 0;
}
```

上面的程序执行结果如下图所示：

![01](../../images/pthread/09.png)

从上面程序的执行结果来看我们设置的 24 MB 的栈空间大小起到了效果，我们可以通过线程的递归次数可以看出来我们确实申请了那么大的空间。在上面的程序当中我们对属性的操作如下，这也是对属性操作的一般流程：

- 使用 `pthread_attr_init` 对属性变量进行初始化操作。
- 使用各种各样的函数对属性 attr 进行操作，比如 `pthread_attr_setstacksize`，这个函数的作用就是用于设置线程的栈空间的大小。
- 使用 `pthread_attr_destroy` 释放线程属性相关的系统资源。

#### 自己为线程的栈申请空间

在上一小节当中我们通过函数 `pthread_attr_setstacksize` 给栈空间设置了新的大小，并且使用程序检查验证了新设置的栈空间大小，在这一小节当中我们将介绍使用我们自己申请的内存空间也可以当作线程的栈使用。我们将使用两种方法取验证这一点：

- 使用 `malloc` 函数申请内存空间，这部分空间主要在堆当中。
- 使用 `mmap` 系统调用在共享库的映射区申请内存空间。

##### 使用 malloc 函数申请内存空间

```c

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define MiB * 1 << 20

int times = 0;
static
void* stack_overflow(void* args) {
  printf("times = %d\n", ++times);
  char s[1 << 20]; // 1 MiB
  stack_overflow(NULL);
  return NULL;
}

int main() {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  void* stack = malloc(2 MiB); // 使用 malloc 函数申请内存空间 申请的空间大小为 2 MiB 
  pthread_t t;
  pthread_attr_setstack(&attr, stack, 2 MiB); // 使用属性设置函数设置栈的位置 栈的最低地址为 stack 栈的大小等于 2 MiB 
  pthread_create(&t, &attr, stack_overflow, NULL);
  pthread_join(t, NULL);
  pthread_attr_destroy(&attr); // 释放系统资源
  free(stack); // 释放堆空间
  return 0;
}
```

上述程序的执行结果如下图所示：

![01](../../images/pthread/10.png)

从上面的执行结果可以看出来我们设置的栈空间的大小为 2MB 成功了。在上面的程序当中我们主要使用 `pthread_attr_setstack` 函数设置栈的低地址和栈空间的大小。我们申请的内存空间内存布局大致如下图所示：

![01](../../images/pthread/11.png)



根据前面知识的学习我们可以知道多个线程的执行流和大致的内存布局如下图所示：

![01](../../images/pthread/02.png)

## 关于栈大小程序的一个小疑惑

```c


#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
int times = 1;

// 先申请内存空间再打印
void* func(void* arg) {
  char s[1 << 20]; // 申请 1MB 内存空间（分配在栈空间上）
  printf("times = %d\n", times);
  times++;
  func(NULL);
  return NULL;
}

int main() {

  pthread_t t;
  pthread_create(&t, NULL, func, NULL);
  pthread_join(t, NULL);
  // func(NULL);
  return 0;
}
```

```c


#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
int times = 1;

// 先打印再申请内存空间
void* func(void* arg) {
  printf("times = %d\n", times);
  times++;
  char s[1 << 20]; // 申请 1MB 内存空间（分配在栈空间上）
  func(NULL);
  return NULL;
}

int main() {

  pthread_t t;
  pthread_create(&t, NULL, func, NULL);
  pthread_join(t, NULL);
  // func(NULL);
  return 0;
}
```



![01](../../images/funny/cat.webp)
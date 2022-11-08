# OpenMP 教程（一）——OpenMP 当中的数据环境

## 前言

在前面的教程[OpenMP入门](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247487188&idx=1&sn=474ac3ef08d47439af963ae4376647a4&chksm=cf0c92ddf87b1bcb969da565e65338829281c7ffc320f0669344dfc7b4fcd4ff7699c55eb1dd&token=1178892963&lang=zh_CN#rd)当中我们简要介绍了 OpenMP 的一些基础的使用方法，在本篇文章当中我们将从一些基础的问题逐渐介绍在 OpenMP 当中和数据的环境相关的指令和子句。

## 从并发求和开始

我们的任务是两个线程同时对一个变量 `data` 进行 `++`操作，执行 10000 次，我们看下面的代码有什么问题：

```c

#include <stdio.h>
#include <omp.h>
#include <unistd.h>

static int data;

int main() {
  #pragma omp parallel num_threads(2) // 使用两个线程同时执行上面的代码块
  {
    for(int i = 0; i < 10000; i++) {
      data++;
      usleep(10);
    }
    // omp_get_thread_num 函数返回线程的 id 号 这个数据从 0 开始，0, 1, 2, 3, 4, ...
    printf("data = %d tid = %d\n", data, omp_get_thread_num());
  }

  printf("In main function data = %d\n", data);
  return 0;
}
```

在上面的代码当中，我们开启了两个线程并且同时执行 `$pragma` 下面的代码块，但是上面的程序有一个问题，就是两个线程可能同时执行 `data++` 操作，但是同时执行这个操作的话，就存在并发程序的数据竞争问题，在 OpenMP 当中默认的数据使用方式就是🧍‍♂️线程之间是共享的比如下面的执行过程：

- 首先线程 1 和线程 2 将 data 加载到 CPU 缓存当中，当前的两个线程得到的 `data` 的值都是 0 。
- 线程 1 和线程 2 对 `data` 进行 ++ 操作，现在两个线程的 `data` 的值都是 1。
- 线程 1 将 data 的值写回到主存当中，那么主存当中的数据的值就等于 1 。
- 线程 2 将 data 的值写回到主存当中，那么主存当中的数据的值也等于 1 。

但是上面的执行过程是存在问题的，因为我们期望的是主存当中的 data 的值等于 2，因此上面的代码是存在错误的。

## 解决求和问题的各种办法

### 使用数组巧妙解决并发程序当中的数据竞争问题

在上面的程序当中我们使用了一个函数 `omp_get_thread_num` 这个函数可以返回线程的 id 号，我们可以根据这个 id 做一些文章，如下面的程序：

```c


#include <stdio.h>
#include <omp.h>
#include <unistd.h>

static int data;

static int tarr[2];

int main() {
  #pragma omp parallel num_threads(2)
  {
    int tid = omp_get_thread_num();
    for(int i = 0; i < 10000; i++) {
      tarr[tid]++;
      usleep(10);
    }
    printf("tarr[%d] = %d tid = %d\n", tid, tarr[tid], tid);
  }
  data = tarr[0] + tarr[1];
  printf("In main function data = %d\n", data);
  return 0;
}
```

在上面的程序当中我们额外的使用了一个数组 `tarr` 用于保存线程的本地的和，然后在最后在主线程里面讲线程本地得到的和相加起来，这样的话我们得到的结果就是正确的了。

```shell
$./lockfree01.out
tarr[1] = 10000 tid = 1
tarr[0] = 10000 tid = 0
In main function data = 20000
```

在上面的程序当中我们需要知道的是，只有当并行域当中所有的线程都执行完成之后，主线程才会继续执行并行域后面的代码，因此主线程在执行代码

```c
  data = tarr[0] + tarr[1];
  printf("In main function data = %d\n", data);
```

之前，OpenMP 中并行域中的代码全部执行完成，因此上面的代码执行的时候数组 `tarr` 中的结果已经计算出来了，因此上面的代码最终的执行结果是 2000。

### reduction 子句

在上文当中我们使用数组去避免多个线程同时操作同一个数据的情况，除了上面的方法处理求和问题，我们还有很多其他方法去解决这个问题，下面我们使用 reduction 子句去解决这个问题：

```c

#include <stdio.h>
#include <omp.h>
#include <unistd.h>

static int data;

int main() {
  #pragma omp parallel num_threads(2) reduction(+:data)
  {
    for(int i = 0; i < 10000; i++) {
      data++;
      usleep(10);
    }
    printf("data = %d tid = %d\n", data, omp_get_thread_num());
  }

  printf("In main function data = %d\n", data);
  return 0;
}
```

在上面的程序当中我们使用了一个子句 `reduction(+:data)` 在每个线程里面对变量 data 进行拷贝，然后在线程当中使用这个拷贝的变量，这样的话就不存在数据竞争了，因为每个线程使用的 data 是不一样的，在 reduction 当中还有一个加号➕，这个加号表示如何进行规约操作，所谓规约操作简单说来就是多个数据逐步进行操作最终得到一个不能够在进行规约的数据。

例如在上面的程序当中我们的规约操作是 + ，因此需要将线程 1 和线程 2 的数据进行 + 操作，即线程 1 的 data 加上 线程 2 的 data 值，然后将得到的结果赋值给全局变量 data，这样的话我们最终得到的结果就是正确的。

如果有 4 个线程的话，那么就有 4 个线程本地的 data（每个线程一个 data）。那么规约（reduction）操作的结果等于：

(((data1 + data2) + data3) + data4) 其中 datai 表示第 i 个线程的得到的 data 。

# OpenMP 教程一——OpenMP 当中的数据环境

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
    printf("data = %d tid = %d\n", data, omp_get_thread_num());
  }

  printf("In main function data = %d\n", data);
  return 0;
}
```

在上面的代码当中，我们开启了两个线程并且同时执行 `$pragma` 下面的代码块，但是上面的程序有一个问题，就是两个线程可能同时执行 `data++` 操作，但是同时执行这个操作的话，就存在并发程序的数据竞争问题，比如下面的执行过程：

- 首先线程 1 和线程 2 将 data 加载到 CPU 缓存当中，当前的两个线程得到的 `data` 的值都是 0 。
- 线程 1 和线程 2 对 `data` 进行 ++ 操作，现在两个线程的 `data` 的值都是 1。
- 线程 1 将 data 的值写回到主存当中，那么主存当中的数据的值就等于 1 。
- 线程 2 将 data 的值写回到主存当中，那么主存当中的数据的值也等于 1 。

但是上面的执行过程是存在问题的，因为我们期望的是主存当中的 data 的值等于 2，因此上面的代码是存在错误的。

## 解决求和问题的各种办法

### 使用数组巧妙解决并发程序当中的数据竞争问题


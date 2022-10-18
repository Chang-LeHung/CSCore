# C语言中这么骚的退出程序的方式你知道几个？

## 前言

在本篇文章当中主要给大家介绍C语言当中一些不常用的特性，比如在`main`函数之前和之后设置我们想要执行的函数，以及各种花式退出程序的方式。

## main函数是最先执行和最后执行的函数吗？

通常我们在写C程序的时候都是从`main`函数开始写，因此我们可能没人有关心过这个问题，事实上是main函数不是程序第一个执行的函数，也不是程序最后一个执行的函数。

```C

#include <stdio.h>

void __attribute__((constructor)) init1() {
  printf("before main funciton\n");
}

int main() {
  printf("this is main funciton\n");
}
```

我们编译上面的代码然后执行，输出结果如下图所示：

```shell
➜  code git:(main) ./init.out 
before main funciton
this is main funciton
```

由此可见main函数并不是第一个被执行的函数，那么程序第一次执行的函数是什么呢？很简单我们看一下程序的调用栈即可。

![01](../../images/programming/01.png)

从上面的结果可以知道，程序第一个执行的函数是`_start`，这是在类Unix操作系统上执行的第一个函数。

那么main函数是程序执行的最后一个函数吗？我们看下面的代码：

```C
#include <stdio.h>

void __attribute__((destructor)) __exit() {
  printf("this is exit\n");
}

void __attribute__((constructor)) init() {
  printf("this is init\n");
}


int main() {
  printf("this is main\n");
  return 0;
}
```

上面程序的输出结果如下：

```shell
➜  code git:(main) ./out.out 
this is init
this is main
this is exit
```

由此可见main函数也不是我们最后执行的函数！事实上我们除了上面的方法之外我们也可以在libc当中注册一些函数，让程序在main函数之后，退出执行前执行这些函数。




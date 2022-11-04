# 你的哪些骚操作会导致Segmentation Fault😂

## 前言

如果你是一个写过一些C程序的同学，那么很大可能你会遇到魔幻的**segmentation fault**，可能一时间抓耳挠腮，本篇文章主要介绍一些常见的导致segmentation fault的代码问题，希望能够帮助大家快速定位问题！

## 出现Segmentation Fault的常见操作

### 写只读数据

```c

#include <stdio.h>

char* str= "hello world";

int main() {
  printf("%s\n", str);
  *str = '1';
  return 0;
}
```

在上面的程序当中，`str`是一个全局变量，一个指向只读数据`hello world`的指针，因为指向的数据存放在只读数据区，如下图所示（rodata区域）：

![03](../../images/programming/03.png)

### 数组下标越界

```c

#include <stdio.h>

int main() {

  int arr[10];
  arr[1 << 20] = 100; // 会导致 segmentation fault
  printf("arr[n] = %d\n", arr[1 << 20]); // 会导致 segmentation fault
  return 0;
}
```

### 栈溢出 stakc_overflow

我们可以使用`ulimit -a`命令查看，系统的一些参数设置，比如说栈的最大大小：

```shell
➜  code git:(main) ✗ ulimit -a
-t: cpu time (seconds)              unlimited
-f: file size (blocks)              unlimited
-d: data seg size (kbytes)          unlimited
-s: stack size (kbytes)             8192
-c: core file size (blocks)         0
-m: resident set size (kbytes)      unlimited
-u: processes                       2061578
-n: file descriptors                1048576
-l: locked-in-memory size (kbytes)  65536
-v: address space (kbytes)          unlimited
-x: file locks                      unlimited
-i: pending signals                 2061578
-q: bytes in POSIX msg queues       819200
-e: max nice                        0
-r: max rt priority                 0
-N 15:                              unlimited
```

上面的参数你可以通过重新编译linux进行更改。在上面的参数当中我们的栈能够申请的最大空间等于`8192kb = 8M`，我们现在写一个程序来测试一下：

```c

#include <stdio.h>

void stakc_overflow(int times) {
  printf("times = %d\n", times);
  char data[1 << 20]; // 每次申请 1 Mega 数据
  stakc_overflow(++times);
}

int main() {

  stakc_overflow(1);
  return 0;
}

```

上面的程序输出结果如下所示：

![03](../../images/programming/04.png)

当我们第8次调用`stakc_overflow`函数的时候，程序崩溃了，因为这个时候我们再申请数组的时候，就一定会超过8M，因为在前面的 7 次调用当中已经申请的 7M 的空间，除此之外还有其他的数据需要使用一定的栈空间，因此会有栈溢出，然后报 segmentation failt 错误。

### 解引用空指针或者野指针

```c
#include <stdio.h>

int main() {

  int* p; 
  printf("%d\n", *p);
  return 0;
}
```

当我们去解引用一个空指针或者一个野指针的时候就汇报segmentation fault，其实本质上还是解引用访问的页面没有分配或者没有权限访问，比如下面代码我们可以解引用一个已经被释放的空间。

```c

#include <stdio.h>
#include <stdint.h>

uint64_t find_rbp() {
  // 这个函数主要是得到寄存器 rbp 的值
  uint64_t rbp;
  asm(
    "movq %%rbp, %0;"
    :"=m"(rbp)::
  );
  return rbp;
}

int main() {

  uint64_t rbp =  find_rbp();
  printf("rbp = %lx\n", rbp);
  // long* p = 0x7ffd4ea724a0;
  printf("%ld\n", *(long*)rbp);
  return 0;
}
```

上面的代码当中我们调用函数 `find_rbp`，得到这个函数对应的寄存器 rbp 的值，当这个函数调用返回的时候，这个函数的栈帧会被摧毁，也就是说 rbp 指向的位置程序已经没有使用了，但是上面的程序不会产生 segmentation fault ，其中最主要的原因就是解引用的位置的页面我们已经分配了，而且我们有读权限，而且我们也有写权限，我们甚至可以给 rbp 指向的位置赋值，像下面那样，程序也不会崩溃。

```c

#include <stdio.h>
#include <stdint.h>

uint64_t find_rbp() {
  uint64_t rbp;
  asm(
    "movq %%rbp, %0;"
    :"=m"(rbp)::
  );
  return rbp;
}

int main() {

  uint64_t rbp =  find_rbp();
  printf("rbp = %lx\n", rbp);
  // long* p = 0x7ffd4ea724a0;
  printf("%ld\n", *(long*)rbp);
  *(long*)rbp = 100; // 给指向的位置进行复制操作
  return 0;
}
```

### 解引用已经释放的内存

```c

#include <stdlib.h>

int main() {
  int* ptr = (int*)malloc(sizeof(int));

  free(ptr);
  *ptr = 10;
  return 0;
}
```

其实上面的代码不见得一定会产生 sementation fault 因为我们使用的libc给我们提供的 free 和 malloc 函数，当我们使用 free 函数释放我们的内存的时候，这部分内存不一定就马上还给操作系统了，因此在地址空间当中这部分内存还是存在的，因此是可以解引用的。

### 其他方式

我相信你肯定还有很多其他的方式去引起 sementation fault 的，嗯相信你！！！😂

造成 sementation failt 的原因主要有以下两大类：

- 读写没有权限的位置：
  - 比如说前面对只读数据区的写操作，或者读写内核数据等等。
- 使用没有分配的页面：
  - 比如数组越界，就是访问一个没有分配的页面。
  - 解引用空指针或者野指针或者没有初始化的指针，因为空指针或者野指针指向的地址没有分配。
  - 不正确的使用解引用和取地址符号，比如你在使用scanf的时候没有使用取地址符号，也可能造成segmentation fault。
  - 栈溢出，这个是操作系统规定的栈的最大空间。

## 总结

在本篇文章当中主要给大家介绍了一些常见的造成段错误的原因，下篇我们仔细分析 segmentation fauilt 的本质，以及我们应该如何应对和处理 segmentation fauilt 。希望大家有所收获～～～

以上就是本篇文章的所有内容了，我是**LeHung**，我们下期再见！！！更多精彩内容合集可访问项目：<https://github.com/Chang-LeHung/CSCore>

关注公众号：**一无是处的研究僧**，了解更多计算机（Java、Python、计算机系统基础、算法与数据结构）知识。

![](../../qrcode2.jpg)


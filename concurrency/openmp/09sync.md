# OpenMP 线程同步 Construct 实现原理以及源码分析（上）

## 前言

在本篇文章当中主要给大家介绍在 OpenMP 当中使用的一些同步的 construct 的实现原理，如 master, single, critical 等等！并且会结合对应的汇编程序进行仔细的分析。（本篇文章的汇编程序分析基于 x86_86 平台）

## Flush Construct

首先先了解一下 flush construct 的语法：

```c
#pragma omp flush(变量列表)
```

这个构造比较简单，其实就是增加一个内存屏障，保证多线程环境下面的数据的可见性，简单来说一个线程对某个数据进行修改之后，修改之后的结果对其他线程来说是可见的。

```c

#include <stdio.h>
#include <omp.h>

int main()
{
  int data = 100;
#pragma omp parallel num_threads(4) default(none) shared(data)
  {
#pragma omp flush(data)
  }
  return 0;
}
```

上面是一个非常简单的 OpenMP 的程序，根据前面的文章 [OpenMp Parallel Construct 实现原理与源码分析](https://github.com/Chang-LeHung/openmp-tutorial/blob/master/docs/parallel.md) 我们可以知道会讲并行域编译成一个函数，我们现在来看一下这个编译后的汇编程序是怎么样的！

gcc-4 编译之后的结果

```asm
00000000004005f6 <main._omp_fn.0>:
  4005f6:       55                      push   %rbp
  4005f7:       48 89 e5                mov    %rsp,%rbp
  4005fa:       48 89 7d f8             mov    %rdi,-0x8(%rbp)
  4005fe:       0f ae f0                mfence 
  400601:       5d                      pop    %rbp
  400602:       c3                      retq   
  400603:       66 2e 0f 1f 84 00 00    nopw   %cs:0x0(%rax,%rax,1)
  40060a:       00 00 00 
  40060d:       0f 1f 00                nopl   (%rax)
```

从上面的结果我们可以看到最终的一条指令是 mfence 这是一条 full 的内存屏障，用于保障数据的可见性，主要是 cache line 中数据的可见性。

gcc-11 编译之后的结果

```asm
0000000000401165 <main._omp_fn.0>:
  401165:       55                      push   %rbp
  401166:       48 89 e5                mov    %rsp,%rbp
  401169:       48 89 7d f8             mov    %rdi,-0x8(%rbp)
  40116d:       f0 48 83 0c 24 00       lock orq $0x0,(%rsp)
  401173:       5d                      pop    %rbp
  401174:       c3                      retq   
  401175:       66 2e 0f 1f 84 00 00    nopw   %cs:0x0(%rax,%rax,1)
  40117c:       00 00 00 
  40117f:       90                      nop
```

从编译之后的结果来看，这个汇编程序主要是使用 lock 指令实现可见性，我们知道 lock 指令是用来保证原子性的，但是事实上这同样也保证来可见性，试想一下如果不保证可见性是不能够保证原子性的！因为如果这个线程看到的数据都不是最新修改的数据的话，那么即使操作是原子的那么也达不到我们想要的效果。

上面两种方式的编译姐若干的主要区别就是一个使用 lock 指令，一个使用 mfence 指令，实际上 lock 的效率比 mfence 效率更高因此在很多场景下，现在都是使用 lock 指令进行实现。

在我的机器上下面的代码分别使用 gcc-11 和 gcc-4 编译之后执行的结果差异很大，gcc-11 大约使用了 11 秒，而 gcc-4 编译出来的结果执行了 20 秒，这其中主要的区别就是 lock 指令和 mfence 指令的差异。

```c

#include <stdio.h>
#include <omp.h>

int main()
{
  double start = omp_get_wtime();
  for(long i = 0; i < 1000000000L; ++i)
  {
    __sync_synchronize();
  }
  printf("time = %lf\n", omp_get_wtime() - start);
  return 0;
}
```

## Master Construct

master construct 的使用方法如下所示：

```c
#pragma omp master
```

事实上编译器会将上面的编译指导语句编译成与下面的代码等价的汇编程序：

```c
if (omp_get_thread_num () == 0)
  block // master 的代码块
```

我们现在来分析一个实际的例子，看看程序编译之后的结果是什么：

```c
#include <stdio.h>
#include <omp.h>

int main()
{
#pragma omp parallel num_threads(4) default(none)
  {
#pragma omp master
    {
      printf("I am master and my tid = %d\n", omp_get_thread_num());
    }
  }
  return 0;
}
```

上面的程序编译之后的结果如下所示（汇编程序的大致分析如下）：

```asm
000000000040117a <main._omp_fn.0>:
  40117a:       55                      push   %rbp
  40117b:       48 89 e5                mov    %rsp,%rbp
  40117e:       48 83 ec 10             sub    $0x10,%rsp
  401182:       48 89 7d f8             mov    %rdi,-0x8(%rbp)
  401186:       e8 a5 fe ff ff          callq  401030 <omp_get_thread_num@plt> # 得到线程的 id 并保存到 eax 寄存器当中
  40118b:       85 c0                   test   %eax,%eax # 看看寄存器 eax 是不是等于 0
  40118d:       75 16                   jne     4011a5  <main._omp_fn.0+0x2b> # 如果不等于 0 则跳转到 4011a5 的位置 也就是直接退出程序了 如果是那么就继续执行后面的 printf 语句
  40118f:       e8 9c fe ff ff          callq  401030 <omp_get_thread_num@plt>
  401194:       89 c6                   mov    %eax,%esi
  401196:       bf 10 20 40 00          mov    $0x402010,%edi
  40119b:       b8 00 00 00 00          mov    $0x0,%eax
  4011a0:       e8 9b fe ff ff          callq  401040 <printf@plt>
  4011a5:       90                      nop
  4011a6:       c9                      leaveq 
  4011a7:       c3                      retq   
  4011a8:       0f 1f 84 00 00 00 00    nopl   0x0(%rax,%rax,1)
  4011af:       00 
```

这里我们只需要了解一下 test 指令就能够理解上面的汇编程序了，"test %eax, %eax" 是 x86 汇编语言中的一条指令，它的含义是对寄存器 EAX 和 EAX 进行逻辑与运算，并将结果存储在状态寄存器中，但是不改变 EAX 的值。这条指令会影响标志位（如 ZF、SF、OF），可用于判断 EAX 是否等于零。

从上面的汇编程序分析我们也可以知道，master construct 就是一条 if 语句，但是后面我们将要谈到的 single 不一样他还需要进行同步。

## Critical Construct

### #pragma omp critical

首先我们需要了解的是 critical 的两种使用方法，在 OpenMP 当中 critical 子句有以下两种使用方法：

```c
#pragma omp critical
#pragma omp critical(name)
```

需要了解的是在 OpenMP 当中每一个 critical 子句的背后都会使用到一个锁，不同的 name 对应不同的锁，如果你使用第一种 critical 的话，那么就是使用 OpenMP 默认的全局锁，需要知道的是同一个时刻只能够有一个线程获得锁，如果你在你的代码当中使用全局的 critical 的话，那么需要注意他的效率，因为在一个时刻只能够有一个线程获取锁。

首先我们先分析第一种使用方式下，编译器会生成什么样的代码，如果我们使用 `#pragma omp critical` 那么在实际的汇编程序当中会使用下面两个动态库函数，GOMP_critical_start 在刚进入临界区的时候调用，GOMP_critical_end 在离开临界区的时候调用。

```c
void GOMP_critical_start (void);
void GOMP_critical_end (void);
```

我们使用下面的程序进行说明：

```c
#include <stdio.h>
#include <omp.h>

int main()
{
  int data = 0;
#pragma omp parallel num_threads(4) default(none) shared(data)
  {
#pragma omp critical
    {
      data++;
    }
  }
  printf("data = %d\n", data);
  return 0;
}
```

根据我们前面的一些文章的分析，并行域在经过编译之后会被编译成一个函数，上面的程序在进行编译之后我们得到如下的结果：

```asm
00000000004011b7 <main._omp_fn.0>:
  4011b7:       55                      push   %rbp
  4011b8:       48 89 e5                mov    %rsp,%rbp
  4011bb:       48 83 ec 10             sub    $0x10,%rsp
  4011bf:       48 89 7d f8             mov    %rdi,-0x8(%rbp)
  4011c3:       e8 b8 fe ff ff          callq  401080 <GOMP_critical_start@plt>
  4011c8:       48 8b 45 f8             mov    -0x8(%rbp),%rax
  4011cc:       8b 00                   mov    (%rax),%eax
  4011ce:       8d 50 01                lea    0x1(%rax),%edx
  4011d1:       48 8b 45 f8             mov    -0x8(%rbp),%rax
  4011d5:       89 10                   mov    %edx,(%rax)
  4011d7:       e8 54 fe ff ff          callq  401030 <GOMP_critical_end@plt>
  4011dc:       c9                      leaveq 
  4011dd:       c3                      retq   
  4011de:       66 90                   xchg   %ax,%ax
```

从上面的反汇编结果来看确实调用了 GOMP_critical_start 和 GOMP_critical_end 两个函数，并且分别是在进入临界区之前和离开临界区之前调用的。在 GOMP_critical_start 函数中会进行加锁操作，在函数 GOMP_critical_end 当中会进行解锁操作，在前面我们已经提到过，这个加锁和解锁操作使用的是 OpenMP 内部的默认的全局锁。

我们看一下这两个函数的源程序：

```c
void
GOMP_critical_start (void)
{
  /* There is an implicit flush on entry to a critical region. */
  __atomic_thread_fence (MEMMODEL_RELEASE);
  gomp_mutex_lock (&default_lock); // default_lock 是一个 OpenMP 内部的锁
}

void
GOMP_critical_end (void)
{
  gomp_mutex_unlock (&default_lock);
}
```

从上面的代码来看主要是调用 gomp_mutex_lock 进行加锁操作，调用 gomp_mutex_unlock 进行解锁操作，这两个函数的内部实现原理我们在前面的文章当中已经进行了详细的解释说明和分析，如果大家感兴趣，可以参考这篇文章 [OpenMP Runtime Library : Openmp 常见的动态库函数使用（下）——深入剖析锁🔒原理与实现](https://github.com/Chang-LeHung/openmp-tutorial/blob/master/docs/runtime02.md) 。

### #pragma omp critical(name)

如果我们使用命令的 critical 的话，那么调用的库函数和前面是不一样的，具体来说是调用下面两个库函数：

```c
void GOMP_critical_name_end (void **pptr);
void GOMP_critical_name_start (void **pptr);
```

其中 pptr 是指向一个指向锁的指针，在前面的文章 [OpenMP Runtime Library : Openmp 常见的动态库函数使用（下）——深入剖析锁🔒原理与实现](https://github.com/Chang-LeHung/openmp-tutorial/blob/master/docs/runtime02.md)  当中我们仔细讨论过这个锁其实就是一个 int 类型的变量。这个变量在编译期间就会在 bss 节分配空间，在程序启动的时候将其初始化为 0 ，表示没上锁的状态，关于这一点在上面谈到的文章当中有仔细的讨论。

这里可能需要区分一下 data 节和 bss 节，.data 节是用来存放程序中定义的全局变量和静态变量的初始值的内存区域。这些变量的值在程序开始执行前就已经确定。.bss 节是用来存放程序中定义的全局变量和静态变量的未初始化的内存区域。这些变量在程序开始执行前并没有初始化的值。在程序开始执行时，这些变量会被系统自动初始化为0。总的来说，.data 存放已初始化数据，.bss存放未初始化数据。

我们现在来分析一个命名的 critical 子句他的汇编程序：

```c
#include <stdio.h>
#include <omp.h>

int main()
{
  int data = 0;
#pragma omp parallel num_threads(4) default(none) shared(data)
  {
#pragma omp critical(A)
    {
      data++;
    }
  }
  printf("data = %d\n", data);
  return 0;
}
```

上面的代码经过编译之后得到下面的结果：

```asm
00000000004011b7 <main._omp_fn.0>:
  4011b7:       55                      push   %rbp
  4011b8:       48 89 e5                mov    %rsp,%rbp
  4011bb:       48 83 ec 10             sub    $0x10,%rsp
  4011bf:       48 89 7d f8             mov    %rdi,-0x8(%rbp)
  4011c3:       bf 58 40 40 00          mov    $0x404058,%edi
  4011c8:       e8 a3 fe ff ff          callq  401070 <GOMP_critical_name_start@plt>
  4011cd:       48 8b 45 f8             mov    -0x8(%rbp),%rax
  4011d1:       8b 00                   mov    (%rax),%eax
  4011d3:       8d 50 01                lea    0x1(%rax),%edx
  4011d6:       48 8b 45 f8             mov    -0x8(%rbp),%rax
  4011da:       89 10                   mov    %edx,(%rax)
  4011dc:       bf 58 40 40 00          mov    $0x404058,%edi
  4011e1:       e8 4a fe ff ff          callq  401030 <GOMP_critical_name_end@plt>
  4011e6:       c9                      leaveq 
  4011e7:       c3                      retq   
  4011e8:       0f 1f 84 00 00 00 00    nopl   0x0(%rax,%rax,1)
```

从上面的结果我们可以看到在调用函数 GOMP_critical_name_start 时，传递的参数的值为 0x404058 （显然这个就是在编译的时候就确定的），我们现在来看一下 0x404058 位置在哪一个节。

根据 x86 的调用规约，rdi/edi 寄存器存储的就是调用函数的第一个参数，而在函数 GOMP_critical_name_start 被调用之前我们可以看到 edi 寄存器的值是 0x404058 ，(`mov $0x404058,%edi`) 因此 pptr 指针的值就是 0x404058 。

为了确定指针指向的数据的位置我们可以查看节头表当中各个节在可执行程序当中的位置，判断 0x404058 在哪个节当中，上面的程序的节头表如下所示：

```shell
Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .interp           PROGBITS         00000000004002a8  000002a8
       000000000000001c  0000000000000000   A       0     0     1
  [ 2] .note.gnu.build-i NOTE             00000000004002c4  000002c4
       0000000000000024  0000000000000000   A       0     0     4
  [ 3] .note.ABI-tag     NOTE             00000000004002e8  000002e8
       0000000000000020  0000000000000000   A       0     0     4
  [ 4] .gnu.hash         GNU_HASH         0000000000400308  00000308
       0000000000000060  0000000000000000   A       5     0     8
  [ 5] .dynsym           DYNSYM           0000000000400368  00000368
       00000000000001e0  0000000000000018   A       6     1     8
  [ 6] .dynstr           STRTAB           0000000000400548  00000548
       0000000000000111  0000000000000000   A       0     0     1
  [ 7] .gnu.version      VERSYM           000000000040065a  0000065a
       0000000000000028  0000000000000002   A       5     0     2
  [ 8] .gnu.version_r    VERNEED          0000000000400688  00000688
       0000000000000050  0000000000000000   A       6     2     8
  [ 9] .rela.dyn         RELA             00000000004006d8  000006d8
       0000000000000018  0000000000000018   A       5     0     8
  [10] .rela.plt         RELA             00000000004006f0  000006f0
       0000000000000090  0000000000000018  AI       5    22     8
  [11] .init             PROGBITS         0000000000401000  00001000
       000000000000001a  0000000000000000  AX       0     0     4
  [12] .plt              PROGBITS         0000000000401020  00001020
       0000000000000070  0000000000000010  AX       0     0     16
  [13] .text             PROGBITS         0000000000401090  00001090
       00000000000001d2  0000000000000000  AX       0     0     16
  [14] .fini             PROGBITS         0000000000401264  00001264
       0000000000000009  0000000000000000  AX       0     0     4
  [15] .rodata           PROGBITS         0000000000402000  00002000
       000000000000001b  0000000000000000   A       0     0     8
  [16] .eh_frame_hdr     PROGBITS         000000000040201c  0000201c
       000000000000003c  0000000000000000   A       0     0     4
  [17] .eh_frame         PROGBITS         0000000000402058  00002058
       0000000000000110  0000000000000000   A       0     0     8
  [18] .init_array       INIT_ARRAY       0000000000403df8  00002df8
       0000000000000008  0000000000000008  WA       0     0     8
  [19] .fini_array       FINI_ARRAY       0000000000403e00  00002e00
       0000000000000008  0000000000000008  WA       0     0     8
  [20] .dynamic          DYNAMIC          0000000000403e08  00002e08
       00000000000001f0  0000000000000010  WA       6     0     8
  [21] .got              PROGBITS         0000000000403ff8  00002ff8
       0000000000000008  0000000000000008  WA       0     0     8
  [22] .got.plt          PROGBITS         0000000000404000  00003000
       0000000000000048  0000000000000008  WA       0     0     8
  [23] .data             PROGBITS         0000000000404048  00003048
       0000000000000004  0000000000000000  WA       0     0     1
  [24] .bss              NOBITS           0000000000404050  0000304c
       0000000000000010  0000000000000000  WA       0     0     8
  [25] .comment          PROGBITS         0000000000000000  0000304c
       000000000000005b  0000000000000001  MS       0     0     1
  [26] .debug_aranges    PROGBITS         0000000000000000  000030a7
       0000000000000030  0000000000000000           0     0     1
  [27] .debug_info       PROGBITS         0000000000000000  000030d7
       0000000000000115  0000000000000000           0     0     1
  [28] .debug_abbrev     PROGBITS         0000000000000000  000031ec
       00000000000000d7  0000000000000000           0     0     1
  [29] .debug_line       PROGBITS         0000000000000000  000032c3
       00000000000000a7  0000000000000000           0     0     1
  [30] .debug_str        PROGBITS         0000000000000000  0000336a
       0000000000000122  0000000000000001  MS       0     0     1
  [31] .symtab           SYMTAB           0000000000000000  00003490
       00000000000003c0  0000000000000018          32    21     8
  [32] .strtab           STRTAB           0000000000000000  00003850
       000000000000023c  0000000000000000           0     0     1
  [33] .shstrtab         STRTAB           0000000000000000  00003a8c
       0000000000000143  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  l (large), p (processor specific)
```

从上面的节头表我们可以看到第 24 个小节 bss 他的起始地址为 0000000000404050 一共站 16 个字节，也就是说 0x404058 指向的数据在 bss 节的数据范围，也就是说锁对应的 int 类型（4 个字节）的数据在 bss 节，程序执行的时候会将 bss 节当中的数据初始化为 0， 0 表示无锁状态。

我们现在来看一下函数 GOMP_critical_name_start 源代码（为了方便查看删除了部分代码）：

```c
void
GOMP_critical_name_start (void **pptr)
{
  gomp_mutex_t *plock;

  /* If a mutex fits within the space for a pointer, and is zero initialized,
     then use the pointer space directly.  */
  if (GOMP_MUTEX_INIT_0
      && sizeof (gomp_mutex_t) <= sizeof (void *)
      && __alignof (gomp_mutex_t) <= sizeof (void *))
    plock = (gomp_mutex_t *)pptr; // gomp_mutex_t 就是 int 类型

  gomp_mutex_lock (plock);
}

```

从语句 `plock = (gomp_mutex_t *)pptr` 可以知道将传递的参数作为一个 int 类型的指针使用，这个指针指向的就是 bss 节的数据，然后对这个数据进行加锁操作（`gomp_mutex_lock (plock)`），关于函数 gomp_mutex_lock ，在文章  [OpenMP Runtime Library : Openmp 常见的动态库函数使用（下）——深入剖析锁🔒原理与实现](https://github.com/Chang-LeHung/openmp-tutorial/blob/master/docs/runtime02.md)  当中有详细的讲解 。

我们在来看一下 GOMP_critical_name_end 的源代码：

```c
void
GOMP_critical_name_end (void **pptr)
{
  gomp_mutex_t *plock;

  /* If a mutex fits within the space for a pointer, and is zero initialized,
     then use the pointer space directly.  */
  if (GOMP_MUTEX_INIT_0
      && sizeof (gomp_mutex_t) <= sizeof (void *)
      && __alignof (gomp_mutex_t) <= sizeof (void *))
    plock = (gomp_mutex_t *)pptr;
  else
    plock = *pptr;

  gomp_mutex_unlock (plock);
}
```

同样的还是使用 bss 节的数据进行解锁操作，关于加锁解锁操作的细节可以阅读这篇文章 [OpenMP Runtime Library : Openmp 常见的动态库函数使用（下）——深入剖析锁🔒原理与实现](https://github.com/Chang-LeHung/openmp-tutorial/blob/master/docs/runtime02.md)  。

## 总结

在本篇文章当中主要给大家介绍了 flush, master 和 critical 指令的实现细节和他的调用的库函数，并且深入分析了这几个 construct 当中设计的库函数的源代码，希望大家有所收获。

---

更多精彩内容合集可访问项目：<https://github.com/Chang-LeHung/CSCore>

关注公众号：一无是处的研究僧，了解更多计算机（Java、Python、计算机系统基础、算法与数据结构）知识。


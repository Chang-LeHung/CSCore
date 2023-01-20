# OpenMP 环境变量使用总结

- OMP_CANCELLATION，在 OpenMP 规范 4.5 当中规定了取消机制，我们可以使用这个环境变量去设置是否启动取消机制，如果这个值等于 TRUE 那么就是开启线程取消机制，如果这个值等于 FALSE 那么就是关闭取消机制。

```c
#include <stdio.h>
#include <omp.h>

int main()
{

   int s = omp_get_cancellation();
   printf("%d\n", s);
#pragma omp parallel num_threads(8) default(none)
   {
      if (omp_get_thread_num() == 2)
      {
#pragma omp cancel parallel
      }

      printf("tid = %d\n", omp_get_thread_num());

   }
   return 0;
}
```

在上面的程序当中，如果我们启动取消机制，那么线程号等于 2 的线程就不会执行后面的 printf 语句。

```shell
➜  cmake-build-hun git:(master) ✗  export OMP_CANCELLATION=TRUE # 启动取消机制
➜  cmake-build-hun git:(master) ✗ ./cancel 
1
tid = 0
tid = 4
tid = 1
tid = 3
tid = 5
tid = 6
tid = 7
```

- OMP_DISPLAY_ENV，这个环境变量的作用就是程序在执行的时候首先会打印 OpenMP 相关的环境变量。如果这个环境变量值等于 TRUE 就会打印环境变量的值，如果是 FLASE 就不会打印。

```shell
➜  cmake-build-hun git:(master) ✗ export OMP_DISPLAY_ENV=TRUE   
➜  cmake-build-hun git:(master) ✗ ./critical 

OPENMP DISPLAY ENVIRONMENT BEGIN
  _OPENMP = '201511'
  OMP_DYNAMIC = 'FALSE'
  OMP_NESTED = 'FALSE'
  OMP_NUM_THREADS = '32'
  OMP_SCHEDULE = 'DYNAMIC'
  OMP_PROC_BIND = 'FALSE'
  OMP_PLACES = ''
  OMP_STACKSIZE = '0'
  OMP_WAIT_POLICY = 'PASSIVE'
  OMP_THREAD_LIMIT = '4294967295'
  OMP_MAX_ACTIVE_LEVELS = '2147483647'
  OMP_CANCELLATION = 'TRUE'
  OMP_DEFAULT_DEVICE = '0'
  OMP_MAX_TASK_PRIORITY = '0'
  OMP_DISPLAY_AFFINITY = 'FALSE'
  OMP_AFFINITY_FORMAT = 'level %L thread %i affinity %A'
OPENMP DISPLAY ENVIRONMENT END
data = 0
```

- OMP_DYNAMIC，如果将这个环境变量设置为true，OpenMP实现可以调整用于执行并行区域的线程数，以优化系统资源的使用。与这个环境变量相关的一共有两个函数：

```c
void omp_set_dynamic(int);
int omp_get_dynamic(void);
```

omp_set_dynamic 使用这个函数表示是否设置动态调整线程的个数，如果传入的参数不等于 0 表示开始，如果参数等于 0 就表示关闭动态调整。

我们现在来谈一谈 dynamic 动态调整线程个数以优化系统资源的使用是什么意思，这个意思就是 OpenMP 创建的线程个数在同一个时刻不会超过你系统的处理器的个数，因为 OpenMP 常常用在数据密集型任务当中，这类任务对 CPU 的需求大，因此为了充分利用资源，只会创建处理器个数的线程个数。

下面我们使用一个例子来验证上面所谈到的内容。

```c
#include <omp.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
//   omp_set_dynamic(1);

#pragma omp parallel num_threads(33) default(none)
   {
      printf("tid = %d\n", omp_get_thread_num());
   }

   return 0;
}
```

上面的代码如果我们没有设置 OMP_DYNAMIC=TRUE 或者没有使用 omp_set_dynamic(1) 去启动态调整的话，那么上面的 printf 语句会被执行 33 次，但是如果你进行了设置，也就是启动了动态调整线程的个数的话，那么创建的线程个数就是 min(33, num_processors) ，后者是你的机器的处理器的个数，比如如果处理器的核的个数是 16 那么就只会有 16 个线程执行并行域当中的代码。

- OMP_NESTED，这个表示是否开启并行域的嵌套模式，这个环境变量要么是 `TRUE` 或者 `FALSE` ，如果这个环境变量的值为 `TRUE` 那么能够嵌套的最大的并行域的数量受到环境变量 OMP_MAX_ACTIVE_LEVELS 的限制，与这个环境变量相关的一个动态库函数为 `void omp_set_nested(int nested);` ，表示是否开启嵌套的并行域。
- OMP_NUM_THREADS，这个表示设置并行域的线程个数，与这个环境变量相关的有 num_threads 这个子句和动态库函数 `void omp_set_num_threads(int num_threads);`也是相关的。他们的优先级为：num_threads > omp_set_num_threads > OMP_NUM_THREADS。这个环境变量的值必须是一个大于 0 的整数，关于他们的优先级你可以认为离并行域越远的就优先级越低，反之越高。
- OMP_STACKSIZE，这个环境变量的主要作用就是设置一个线程的栈空间的大小。
- OMP_WAIT_POLICY，这个参数的主要作用就是控制当线程没有拿到锁的时候是自旋获取锁还是进入内核被挂起。这个参数主要有两个值，active 或者 passive。
  - PASSIVE，等待的线程不消耗 CPU ，而是进入内核挂起。
  - ACTIVE，等待的线程消耗 CPU，一直自旋获取锁。

我们现在使用例子来验证上面的规则：

```c
#include <stdio.h>
#include <omp.h>

int main()
{
   omp_lock_t lock;
   omp_init_lock(&lock);
#pragma omp parallel num_threads(16) default(none) shared(lock)
   {
      omp_set_lock(&lock);
      while (1);
      omp_unset_lock(&lock);
   }
   return 0;
}
```

在上面的代码当中有一个并行域，并行域中线程的个数是 16，我们首先使用 ACTIVE 来看一下这个进程的负载，根据前面我们的描述那么 16 个线程都会在自旋获取锁，这个过程将会一直使用 CPU，因此这个进程的负载 %CPU ，应该是接近 1600 % ，每个线程都是 100% 加起来就是 1600 % 。

```shell
➜  cmake-build-openmp export OMP_WAIT_POLICY=ACTIVE 
➜  cmake-build-openmp ./wait_policy                
```

我们使用 top 命令查看一下这个进程的 CPU 使用率。

```shell
top - 17:27:14 up 263 days,  2:11,  2 users,  load average: 93.87, 87.59, 85.78
Tasks:  31 total,   2 running,  29 sleeping,   0 stopped,   0 zombie
%Cpu(s): 80.0 us,  0.7 sy,  0.0 ni, 19.2 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
KiB Mem : 13191648+total, 54673112 free, 15049648 used, 62193724 buff/cache
KiB Swap: 12499968+total, 11869649+free,  6303184 used. 11600438+avail Mem

   PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
112290 root      20   0  133868   1576   1452 R  1600  0.0  11:52.84 wait_policy
```

根据上面的输出结果我们可以看到我们的预测是对的，所有的线程都活跃的在使用 CPU。

现在我们再来看一下如果我们使用 PASSIVE 的情况会是怎么样的？根据前面的描述如果线程没有获取到锁那么就会被挂起，因为只能够有一个线程获取到锁，其余 15 个线程都将被挂起，因此 CPU 的使用率应该是  100 % 左右，这个线程就是那个获取到锁的线程。

```shell
➜  cmake-build-openmp export OMP_WAIT_POLICY=PASSIVE
➜  cmake-build-openmp ./wait_policy 
```

我们再使用 top 命令查看一下对应的输出：

```shell
top - 17:27:53 up 263 days,  2:11,  2 users,  load average: 92.76, 88.10, 86.03
Tasks:  31 total,   2 running,  29 sleeping,   0 stopped,   0 zombie
%Cpu(s): 53.3 us,  0.8 sy,  0.0 ni, 45.9 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
KiB Mem : 13191648+total, 54675824 free, 15046932 used, 62193728 buff/cache
KiB Swap: 12499968+total, 11869649+free,  6303184 used. 11600710+avail Mem

   PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
112317 root      20   0  133868   1624   1496 R  99.3  0.0   0:04.58 wait_policy
```

从上面的输出结果来看也是符合我们的预期，只有一个线程在不断的使用 CPU。

- GOMP_SPINCOUNT，这个环境变量的主要作用就是当 OMP_WAIT_POLICY 是 active 的时候，最多忙等待自旋多少次，如果自旋的次数超过这个值的话，那么这个线程将会被挂起。

  当这个环境变量没有定义：

  - OMP_WAIT_POLICY=PASSIVE，那么自旋次数为 0 。
  - 如果 OMP_WAIT_POLICY 也是未定义的话，那么这个自旋次数将会被设置成 300,000 。
  - OMP_WAIT_POLICY=ACTIVE，那么自旋的次数是 300 亿次。

  另外如果 OpenMP 的线程的个数大于可用的 CPU 的核心的个数的时候，1000 和 100 次就是 GOMP_SPINCOUNT 的值，对应OMP_WAIT_POLICY=ACTIVE 和 OMP_WAIT_POLICY 没有定义。

- OMP_MAX_TASK_PRIORITY，这个是设置 OpenMP 任务的优先级的最大值，这个值应该是一个大于等于 0 的值，如果没有定义，默认优先级的值就是 0 。

- OMP_MAX_ACTIVE_LEVELS，这个参数的主要作用是设置最大的嵌套的并行域的个数。

- GOMP_CPU_AFFINITY，这个环境变量的作用就是将线程绑定到特定的 CPU 核心上。该变量应包含以空格分隔或逗号分隔的CPU列表。此列表可能包含不同类型的条目：任意顺序的单个CPU编号、CPU范围（M-N）或具有一定步长的范围（M-N:S）。CPU编号从零开始。例如，GOMP_CPU_AFFINITY=“0 3 1-2 4-15:2”将分别将初始线程绑定到CPU 0，第二个绑定到CPU 3，第三个绑定到CPU1，第四个绑定到CPU 2，第五个绑定到CPU 4，第六个到第十个绑定到ccu 6、8、10、12和14，然后从列表的开头开始重新分配。GOMP_CPU_AFFINITY=0将所有线程绑定到CPU 0。

我们现在来使用一个例子查看环境变量的使用。我们的测试程序如下：

```c
#include <stdio.h>
#include <omp.h>

int main()
{
   omp_lock_t lock;
   omp_init_lock(&lock);
#pragma omp parallel num_threads(4) default(none) shared(lock)
   {
      while (1);
   }
   return 0;
}
```

上面的程序就是开启四个线程然后进行死循环。在我的测试环境中一共有 4 个 CPU 计算核心。我们现在执行上面的程序，对应的结果如下所示，下面的图是使用命令 htop 得到的结果：

```shell
➜  tmp ./a.out
────────────────────────────────────────────────────────────────────────────────

    0[||||||||||||||||||||||||100.0%]   Tasks: 118, 212 thr; 4 running
    1[||||||||||||||||||||||||100.0%]   Load average: 2.62 0.86 0.29
    2[||||||||||||||||||||||||100.0%]   Uptime: 04:21:10
    3[||||||||||||||||||||||||100.0%]
  Mem[||||||||||||||||||||575M/3.82G]
  Swp[                      0K/3.82G]

    PID USER      PRI  NI  VIRT   RES   SHR S CPU%▽MEM%   TIME+  Command
  10750 lehung     20   0 27304   852   756 R 400.  0.0  2:30.53 ./a.out
```

从上面 htop 命令的输出结果可以看到 0 - 3 四个核心都跑满了，我们现在来看一下如果我们使用 GOMP_CPU_AFFINITY 环境变量使用线程绑定的方式 CPU 的负载将会是什么样！下面我们将所有的线程绑定到 0 1 两个核心，那么根据我们之前的分析 0 号核心上将会有第一个和第三个线程，1 号核心将会有第二个和第四个线程在上面运行。

```shell
➜  tmp export GOMP_CPU_AFFINITY="0 1"
➜  tmp ./a.out
────────────────────────────────────────────────────────────────────────────────

    0[||||||||||||||||||||||||100.0%]   Tasks: 118, 213 thr; 4 running
    1[||||||||||||||||||||||||100.0%]   Load average: 2.29 1.10 0.41
    2[|                         1.3%]   Uptime: 04:22:03
    3[|                         0.7%]
  Mem[||||||||||||||||||||576M/3.82G]
  Swp[                      0K/3.82G]

    PID USER      PRI  NI  VIRT   RES   SHR S CPU%▽MEM%   TIME+  Command
  10772 lehung     20   0 27304   840   744 R 200.  0.0  0:10.42 ./a.out
```

其实与上面的过程相关的两个主要的系统调用就是：

```c
int sched_setaffinity(pid_t pid, size_t cpusetsize,
                             const cpu_set_t *mask);
int sched_getaffinity(pid_t pid, size_t cpusetsize,
                             cpu_set_t *mask);
```

感兴趣的同学可能查看一下上面的两个函数的手册。

- OMP_SCHEDULE，这个环境变量主要是用在 OpenMP 关于 for 循环的调度上的，他的规则为 OMP_SCHEDULE=type[,chunk]，其中 type 的取值可以为 static, dynamic, guided, auto 。并且 chunk size 是可选的，而且他的值是一个正整数。如果这个环境变量没有定义，默认的调度方式是 dynamic 并且 chunk size = 1 。

## 总结

在本篇文章当中主要给大家介绍了一些经常使用的 OpenMP 系统环境变量，设置环境变量有时候能够更加方便的设置程序，同时有些环境变量对应一些 OpenMP 的动态库函数。以上就是本篇文章的所有内容希望大家有所收获！

---

更多精彩内容合集可访问项目：<https://github.com/Chang-LeHung/CSCore>

关注公众号：一无是处的研究僧，了解更多计算机（Java、Python、计算机系统基础、算法与数据结构）知识。


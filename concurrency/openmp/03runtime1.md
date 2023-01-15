# Openmp Runtime 库函数汇总（上）

- omp_in_parallel，如果当前线程正在并行域内部，则此函数返回true，否则返回false。

```c
#include <stdio.h>
#include <omp.h>

int main()
{
   printf("0> Not In parallel region value = %d\n",
          omp_in_parallel());

   // 在这里函数返回的 false 在 C 语言当中返回值等于 int 类型的 0
   if (omp_in_parallel())
   {
      printf("1> In parallel region value = %d\n",
             omp_in_parallel());
   }
   #pragma omp parallel num_threads(2) default(none)
   {
      // 这里函数的返回值是 1 在 C 语言当中对应 int 类型的 1
      if (omp_in_parallel())
      {
         printf("2> In parallel region value = %d\n",
                omp_in_parallel());
      }
   }
   return 0;
}
```

上面的程序的输出结果如下所示：

```shell
0> Not In parallel region value = 0
2> In parallel region value = 1
2> In parallel region value = 1
```

需要注意的是在上面的函数 omp_in_parallel 使用时需要注意，如果你的并行域只有一个线程的时候 omp_in_parallel 返回的是 false。比如下面的例子：

```c
#include <stdio.h>
#include <omp.h>

int main()
{
   printf("0> Not In parallel region value = %d\n",
          omp_in_parallel());

   // 在这里函数返回的 false 在 C 语言当中返回值等于 int 类型的 0
   if (omp_in_parallel())
   {
      printf("1> In parallel region value = %d\n",
             omp_in_parallel());
   }
  // 只有一个线程，因此并不会激活并行域
   #pragma omp parallel num_threads(1) default(none)
   {
      // 这里函数的返回值是 1 在 C 语言当中对应 int 类型的 1
      if (omp_in_parallel())
      {
         printf("2> In parallel region value = %d\n",
                omp_in_parallel());
      }
   }
   return 0;
}
```

在上面的程序当中，因为并行域当中只有一个线程，因此不会激活，所以即使在 parallel 代码块内不，这个 omp_in_parallel 也不会被触发，上面的程序只会输出第一个 printf 的内容：

```shell
0> Not In parallel region value = 0
```

- omp_get_thread_num，这个函数的主要作用是用于返回当前并行域的线程组当中的唯一 ID，在一个串行代码当中，这个函数的返回结果之中等于 0 ，在并行域当中这个函数的返回值的区间等于 [0, num_threads - 1]，如果是一个线程组的 master 线程调用这个函数，这个函数的返回值始终是 0，对应的测试代码如下所示：

```c
#include <stdio.h>
#include <omp.h>

int main()
{
   printf("Non parallel region: omp_get_thread_num() = %d\n", omp_get_thread_num());

#pragma omp parallel num_threads(5) default(none)
   {
      printf("In parallel region: omp_get_thread_num() = %d\n", omp_get_thread_num());
#pragma omp master
      {
         printf("In parallel region master: omp_get_thread_num() = %d\n", omp_get_thread_num());
      }
   }

   return 0;
}
```

上面的程序的输出结果如下所示：

```shell
Non parallel region: omp_get_thread_num() = 0
In parallel region: omp_get_thread_num() = 0
In parallel region master: omp_get_thread_num() = 0
In parallel region: omp_get_thread_num() = 4
In parallel region: omp_get_thread_num() = 1
In parallel region: omp_get_thread_num() = 2
In parallel region: omp_get_thread_num() = 3
```

在上面的代码当中我们可以看到，在非并行域当中（第一个 printf） 函数的输出结果是 0，在并行域当中程序的输出结果范围是 [0. num_threads]，如果一个线程是 master 线程的话，那么它对应的线程组当中的返回值就是0。

- omp_get_team_size，这个函数的主要作用就是返回一个线程组当中的线程个数。这个函数接受一个参数，他的函数原型为 `int omp_get_team_size(int level);`，这个参数的 level 的含义就是并行域的嵌套层级，这个参数的范围是 [0, omp_get_level]，如果传入的参数不在这个范围，那么这个函数的返回值为 -1，如果你在一个并行域传入的 level 的参数是  omp_get_level ，那么这个函数的返回值和 omp_get_num_threads 的返回值相等。

```c
#include <stdio.h>
#include <omp.h>

int main()
{
   omp_set_nested(1);
   int level = omp_get_level();
   int size = omp_get_team_size(level);
   printf("Non parallel region : level = %d size = %d\n", level, size);

#pragma omp parallel num_threads(5) default(none)
   {
      int level = omp_get_level();
      int size = omp_get_team_size(level);
      printf("level = %d size = %d\n", level, size);

#pragma omp parallel num_threads(2) default(none)
      {
         int level = omp_get_level();
         int size = omp_get_team_size(level);
         printf("level = %d size = %d\n", level, size);

         printf("Up one level team size = %d\n", omp_get_team_size(level - 1));
      }
   }
   return 0;
}
```

上面的程序的输出结果如下所示：

```shell
Non parallel region : level = 0 size = 1
level = 1 size = 5
level = 1 size = 5
level = 1 size = 5
level = 1 size = 5
level = 1 size = 5
level = 2 size = 2
Up one level team size = 5
level = 2 size = 2
Up one level team size = 5
level = 2 size = 2
Up one level team size = 5
level = 2 size = 2
level = 2 size = 2
Up one level team size = 5
level = 2 size = 2
Up one level team size = 5
level = 2 size = 2
Up one level team size = 5
Up one level team size = 5
level = 2 size = 2
Up one level team size = 5
level = 2 size = 2
Up one level team size = 5
level = 2 size = 2
Up one level team size = 5
```

在上面的代码当中在非并行域的代码当中，程序的输出结果和我们上面谈到的是相同的，level 等于 0，team size 等于 1，在并行域当中输出的结果也是符合预期的，我们来看一下最后两个输出，倒数第一个输出是查看上一个 level 的输出结果，可以看到他的输出结果等于 5，这和我们设置的 5 个线程是相符的。

- omp_get_num_procs，这个函数相对比较简单，主要是返回你的系统当中处理器的数量。

```c
#include <stdio.h>
#include <omp.h>

int main()
{
   int nprocs = omp_get_num_procs();
   printf("Number of processors = %d\n", nprocs);
   return 0;
}
```

- omp_in_final，这个函数主要是查看当前线程是否在最终任务或者包含任务当中，比如在下面的例子当中，我们就可以测试这个函数：

```c
#include <stdio.h>
#include <omp.h>

int echo(int n)
{
   int ret;
#pragma omp task shared(ret, n) final(n <= 10)
   {
      if(omp_in_final())
      {
         printf("In final n = %d\n", n);
         ret = n;
      }
      else
      {
         ret = 1 + echo(n - 1);
      }
   }
   return ret;
}

int main()
{
   printf("echo(100) = %d\n", echo(100));
   return 0;
}
```

上面的程序的输出结果如下所示：

```shell
In final n = 10
echo(100) = 100
```

在上面的程序我们在函数 echo 当中定义了一个任务，当参数 n <= 10 的时候，这个任务会变成一个 final 任务，因此在 n == 10 的时候，这个函数的 omp_in_final 会返回 true，因此会直接将 10 赋值给 ret ，不会进行递归调用，因此我们得到了上面的输出结果。

- omp_get_nested ，这个函数是用于判断是否开启并行区域的嵌套，如果开启了并行区域的嵌套，那么这个函数就返回 true ，否则就返回 false ，对应 C 语言的值分别为 1 和 0。在 OpenMP 当中默认是关闭的，你可以使用函数 omp_set_nested(1) 进行开启，或者使用 omp_set_nested(0) 关闭并行区域的嵌套。你可以对并行区域的嵌套有所疑惑，我们来看下面两个例子，你就豁然开朗了。

```c

#include <stdio.h>
#include <omp.h>


int main()
{
//   omp_set_nested(1);
   #pragma omp parallel num_threads(2) default(none)
   {
      printf("Outer tid = %d level = %d num_threads = %d\n",
             omp_get_thread_num(), omp_get_active_level(),
             omp_get_num_threads());
      #pragma omp parallel num_threads(2) default(none)
      {
         printf("Inner tid = %d level = %d num_threads = %d\n",
                omp_get_thread_num(), omp_get_active_level(),
                omp_get_num_threads());
      }
   }
   return 0;
}
```

上面的程序的输出结果如下所示：

```shell
Outer tid = 0 level = 1 num_threads = 2
Inner tid = 0 level = 1 num_threads = 1
Outer tid = 1 level = 1 num_threads = 2
Inner tid = 0 level = 1 num_threads = 1
```

我们再来看一下如果我们开启了并行区域的嵌套之后程序的输出结果：

```c

#include <stdio.h>
#include <omp.h>


int main()
{
   omp_set_nested(1);
   #pragma omp parallel num_threads(2) default(none)
   {
      printf("Outer tid = %d level = %d num_threads = %d\n",
             omp_get_thread_num(), omp_get_active_level(),
             omp_get_num_threads());
      #pragma omp parallel num_threads(2) default(none)
      {
         printf("Inner tid = %d level = %d num_threads = %d\n",
                omp_get_thread_num(), omp_get_active_level(),
                omp_get_num_threads());
      }
   }
   return 0;
}
```

上面的程序的输出结果如下所示：

```shell
Outer tid = 0 level = 1 num_threads = 2
Outer tid = 1 level = 1 num_threads = 2
Inner tid = 0 level = 2 num_threads = 2
Inner tid = 1 level = 2 num_threads = 2
Inner tid = 0 level = 2 num_threads = 2
Inner tid = 1 level = 2 num_threads = 2
```

我们可以对比一下两个程序的输出结果的差异，我们可以看到当我们允许并行嵌套之后，程序的输出结果会更多一点，这是因为如果我们没有开启的话，子并行域每个线程只会启动一个线程，如果我们开启的话，那么每个线程就会重新启动 num_threads 个线程，比如在上面两个代码当中，对于第一份代码外部并行块会启动两个线程，然后在每个线程的内部有一个并行块，但是因为没有启动，因此不管你的 num_threads 设置成多少，每个线程只会启动一个线程，因此最终会有 4 个 printf 语句输出。而对于第二份代码，是启动了嵌套代码的，外部并行块有两个线程，在内部因为是开启了，这两个线程会分别创建 num_threads 个内部线程去执行内部代码块，因此会有 2 个外部线程 4 （2x2）个内部线程，一共会有 6 个输出，因此就符合上面的结果了。其实不仅可以嵌套两层，还可以嵌套更多层，原理也是一样的分析方法。

- omp_get_active_level，这个函数主要是返回当前线程所在的并行域的嵌套的并行块的级别，从 1 开始，从外往内没加一层这个值就加一，如果没有启动并行嵌套，那么这个函数的返回值就是 1。

- omp_set_max_active_levels，我们在上面提到了，我们可以不断的进行并行域的嵌套，我们可以使用这个函数进行设置最大的嵌套的层数，如果超过这个层数那么就与不启动嵌套的效果一样了，也就是说 num_threads 不会发生效果了。

我们现在设置允许设置的最大的并行块的嵌套层数等于 2，但是我们一共有三个嵌套块，我们可以看一下对第三层嵌套的代码的影响。

```c
#include <stdio.h>
#include <omp.h>

int main()
{
   omp_set_nested(1);
   omp_set_max_active_levels(2);

#pragma omp parallel num_threads(2) default(none)
   {
      printf("1> omp_get_level = %d omp_get_active_level = %d\n", omp_get_level(), omp_get_active_level());
#pragma omp parallel num_threads(2) default(none)
      {
         printf("2> omp_get_level = %d omp_get_active_level = %d\n", omp_get_level(), omp_get_active_level());
#pragma omp parallel num_threads(2) default(none)
         {
            printf("3> omp_get_level = %d omp_get_active_level = %d\n", omp_get_level(), omp_get_active_level());

         }
      }
   }
   return 0;
}
```

上面的程序的输出结果如下所示（3> 一共输出了 4 次，因为它的外部线程的个数是 4 (2x2)）：

```shell
1> omp_get_level = 1 omp_get_active_level = 1
1> omp_get_level = 1 omp_get_active_level = 1
2> omp_get_level = 2 omp_get_active_level = 2
3> omp_get_level = 3 omp_get_active_level = 2
2> omp_get_level = 2 omp_get_active_level = 2
2> omp_get_level = 2 omp_get_active_level = 2
3> omp_get_level = 3 omp_get_active_level = 2
2> omp_get_level = 2 omp_get_active_level = 2
3> omp_get_level = 3 omp_get_active_level = 2
3> omp_get_level = 3 omp_get_active_level = 2
```

我们现在将函数 omp_set_max_active_levels 的参数设置成 3，也就是说允许的最大的并行块的嵌套层数等于3，重新看一下程序的输出结果是什么（将 omp_set_max_active_levels(2) 改成 omp_set_max_active_levels(3)）之后，程序的输出结果如下所示：

```shell
1> omp_get_level = 1 omp_get_active_level = 1
1> omp_get_level = 1 omp_get_active_level = 1
2> omp_get_level = 2 omp_get_active_level = 2
2> omp_get_level = 2 omp_get_active_level = 2
2> omp_get_level = 2 omp_get_active_level = 2
2> omp_get_level = 2 omp_get_active_level = 2
3> omp_get_level = 3 omp_get_active_level = 3
3> omp_get_level = 3 omp_get_active_level = 3
3> omp_get_level = 3 omp_get_active_level = 3
3> omp_get_level = 3 omp_get_active_level = 3
3> omp_get_level = 3 omp_get_active_level = 3
3> omp_get_level = 3 omp_get_active_level = 3
3> omp_get_level = 3 omp_get_active_level = 3
3> omp_get_level = 3 omp_get_active_level = 3
```

可以看到 3> 的输出次数等于8，这个次数就表明外部 4 个线程都在第三个并行块产生了两个线程，这就是启动嵌套并行块的效果，这就是设置函数 omp_set_max_active_levels 的效果。

- omp_get_nested，这个函数的作用就是返回是否启动了并行嵌套，在 C 语言当中如果启动了那么就是返回 1，否则就是返回 0。

```c
#include <stdio.h>
#include <omp.h>

int main()
{
   printf("omp_get_nested() = %d\n", omp_get_nested());
   omp_set_nested(1);
   printf("omp_get_nested() = %d\n", omp_get_nested());
   return 0;
}
```

上面的程序的输出结果如下所示：

```shell
omp_get_nested() = 0
omp_get_nested() = 1
```

## 总结

在本篇文章当中主要给大家介绍了一些在 OpenMP 当中常用的动态库函数，这篇文章的动态库函数主要是关于并行域和线程状态的函数，在下篇文章当中我们主要是分析一些 OpenMP 当中的锁相关函数。希望大家有所收获！

---

更多精彩内容合集可访问项目：<https://github.com/Chang-LeHung/CSCore>

关注公众号：一无是处的研究僧，了解更多计算机（Java、Python、计算机系统基础、算法与数据结构）知识。

![](https://img2022.cnblogs.com/blog/2519003/202207/2519003-20220703200459566-1837431658.jpg)


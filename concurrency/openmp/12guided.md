# OpenMP For Construct guided 调度方式实现原理和源码分析

## 前言

在本篇文章当中主要给大家介绍在 OpenMP 当中 guided 调度方式的实现原理。这个调度方式其实和 dynamic 调度方式非常相似的，从编译器角度来说基本上是一样的，在本篇文章当中就不介绍一些相关的必备知识了，如果不了解可以先看这篇文章 [OpenMP For Construct dynamic 调度方式实现原理和源码分析](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247487775&idx=1&sn=112f5fb600584bdd4a7acfeddb58cd6e&chksm=cf0c8d16f87b040098e650d350dce82b1fa75549c05ba25d92c8a4da3da661cfcef5a5c9542c&token=1250927684&lang=zh_CN#rd) 。

## guided 调度方式分析

我们使用下面的代码来分析一下 guided 调度的情况下整个程序的执行流程是怎么样的：

```c
#pragma omp parallel for num_threads(t) schedule(dynamic, size)
for (i = lb; i <= ub; i++)
  body;
```

编译器会将上面的程序编译成下面的形式：

```c
void subfunction (void *data)
{
  long _s0, _e0;
  while (GOMP_loop_guided_next (&_s0, &_e0))
  {
    long _e1 = _e0, i;
    for (i = _s0; i < _e1; i++)
      body;
  }
  // GOMP_loop_end_nowait 这个函数的主要作用就是释放数据的内存空间 在后文当中不进行分析
  GOMP_loop_end_nowait ();
}

GOMP_parallel_loop_guided_start (subfunction, NULL, t, lb, ub+1, 1, size);
subfunction (NULL);
// 这个函数在前面的很多文章已经分析过 本文也不在进行分析
GOMP_parallel_end ();
```

根据上面的代码可以知道，上面的代码当中最主要的两个函数就是 GOMP_parallel_loop_guided_start 和 GOMP_loop_guided_next，现在我们来分析一下他们的源代码：

- GOMP_parallel_loop_guided_start

```c
void
GOMP_parallel_loop_guided_start (void (*fn) (void *), void *data,
				 unsigned num_threads, long start, long end,
				 long incr, long chunk_size)
{
  gomp_parallel_loop_start (fn, data, num_threads, start, end, incr,
			    GFS_GUIDED, chunk_size);
}

static void
gomp_parallel_loop_start (void (*fn) (void *), void *data,
			  unsigned num_threads, long start, long end,
			  long incr, enum gomp_schedule_type sched,
			  long chunk_size)
{
  struct gomp_team *team;
	// 解析到底启动几个线程执行并行域的代码
  num_threads = gomp_resolve_num_threads (num_threads, 0);
  // 创建线程组
  team = gomp_new_team (num_threads);
  // 对共享数据进行初始化操作
  gomp_loop_init (&team->work_shares[0], start, end, incr, sched, chunk_size);
  // 启动线程组执行函数 fn
  gomp_team_start (fn, data, num_threads, team);
}

```

在上面的程序当中 GOMP_parallel_loop_guided_start，有 7 个参数，我们接下来仔细解释一下这七个参数的含义：

- fn，函数指针也就是并行域被编译之后的函数。
- data，指向共享或者私有的数据，在并行域当中可能会使用外部的一些变量。
- num_threads，并行域当中指定启动线程的个数。
- start，for 循环迭代的初始值，比如 for(int i = 0; ;) 这个 start 就是 0 。
- end，for 循环迭代的最终值，比如 for(int i = 0; i < 100; i++) 这个 end 就是 100 。
- incr，这个值一般都是 1 或者 -1，如果是 for 循环是从小到达迭代这个值就是 1，反之就是 -1，实际上这个值指的是 for 循环 i 大的增量。
- chunk_size，这个就是给一个线程划分块的时候一个块的大小，比如 schedule(dynamic, 1)，这个 chunk_size 就等于 1 。

事实上上面的代码和 GOMP_parallel_loop_dynamic_start 基本上一摸一样，函数参数也一直，唯一的区别就是调度方式的不同，上面的代码和前面的文章 [OpenMP For Construct dynamic 调度方式实现原理和源码分析](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247487775&idx=1&sn=112f5fb600584bdd4a7acfeddb58cd6e&chksm=cf0c8d16f87b040098e650d350dce82b1fa75549c05ba25d92c8a4da3da661cfcef5a5c9542c&token=1250927684&lang=zh_CN#rd) 基本一样因此不再进行详细的分析。

- GOMP_loop_guided_next，这是整个 guided 调度方式的核心代码（整个过程仍然使用 CAS 进行原子操作，保证并发安全）

```c
static bool
gomp_loop_guided_next (long *istart, long *iend)
{
  bool ret;
  ret = gomp_iter_guided_next (istart, iend);
  return ret;
}

bool
gomp_iter_guided_next (long *pstart, long *pend)
{
  struct gomp_thread *thr = gomp_thread ();
  struct gomp_work_share *ws = thr->ts.work_share;
  struct gomp_team *team = thr->ts.team;
  unsigned long nthreads = team ? team->nthreads : 1;
  long start, end, nend, incr;
  unsigned long chunk_size;
	
  // 下一个分块的起始位置
  start = ws->next;
  // 最终位置 不能够超过这个位置
  end = ws->end;
  incr = ws->incr;
  // chunk_size 是每个线程的分块大小
  chunk_size = ws->chunk_size;

  while (1)
    {
      unsigned long n, q;
      long tmp;
			// 如果下一个分块的起始位置等于最终位置 那就说明没有需要继续分块的了 因此返回 false 表示没有分块需要执行了 
      if (start == end)
	return false;
    	// 下面就是整个划分的逻辑 大家可以吧 incr = 1 带入 就能够知道每次线程分得的数据就是当前剩下的数据处以线程的个数
      n = (end - start) / incr;
      q = (n + nthreads - 1) / nthreads;

      if (q < chunk_size)
	q = chunk_size;
      if (__builtin_expect (q <= n, 1))
	nend = start + q * incr;
      else
	nend = end;
    	// 进行比较并交换操作 比较 start 和 ws->next 的值，如果相等则将 ws->next 的值变为 nend 并且返回 ws->next 原来的值
      tmp = __sync_val_compare_and_swap (&ws->next, start, nend);
      if (__builtin_expect (tmp == start, 1))
	break;

      start = tmp;
    }

  *pstart = start;
  *pend = nend;
  return true;
}
```

从上面的整个分析过程来看，guided 调度方式之所以每个线程的分块呈现递减趋势，是因为每次执行完一个 chunk size 之后，剩下的总的数据就少了，然后又除以线程数，因此每次得到的 chunk size 都是单调递减的。

## 总结

在本篇文章当中主要介绍了 OpenMP 当中 guided 调度方式当中数据的划分策略以及具体的实现代码， OpenMP 当中 for 循环的几种调度策略的越代码是非常相似的，只有具体的划分策略的 xxx_next 代码实现不同，因此整体来说是相对比较好阅读的。guided 调度方式主要是用剩下的数据个数除以线程的个数就是线程所得到的 chunk size 的大小，然后更新剩下的数据个数再次除以线程的个数就是下一个线程所得到的 chunk size 大小，如此反复直到划分完成。

---

更多精彩内容合集可访问项目：<https://github.com/Chang-LeHung/CSCore>

关注公众号：一无是处的研究僧，了解更多计算机（Java、Python、计算机系统基础、算法与数据结构）知识。


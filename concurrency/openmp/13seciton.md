# OpenMP Sections Construct 实现原理以及源码分析

## 前言

在本篇文章当中主要给大家介绍 OpenMP 当中主要给大家介绍 OpenMP 当中 sections construct 的实现原理以及他调用的动态库函数分析。如果已经了解过了前面的关于 for 的调度方式的分析，本篇文章就非常简单了。

## 编译器角度分析

在这一小节当中我们将从编译器角度去分析编译器会怎么处理 sections construct ，我们以下面的 sections construct 为例子，看看编译器是如何处理 sections construct 的。

```c
#pragma omp sections
{
  #pragma omp section
  stmt1;
  #pragma omp section
  stmt2;
  #pragma omp section
  stmt3;
}
```

上面的代码会被编译器转换成下面的形式，其中 GOMP_sections_start 和 GOMP_sections_next 是并发安全的，他们都会返回一个数据表示第几个 omp section 代码块，其中 GOMP_sections_start 的参数是表示有几个 omp section 代码块，并且返回给线程一个整数表示线程需要执行第几个 section 代码块，这两个函数的意义不同的是在 GOMP_sections_start 当中会进行一些数据的初始化操作。当两个函数返回 0 的时候表示所有的 section 都被执行完了，从而退出 for 循环。

```c
for (i = GOMP_sections_start (3); i != 0; i = GOMP_sections_next ())
  switch (i)
    {
    case 1:
      stmt1;
      break;
    case 2:
      stmt2;
      break;
    case 3:
      stmt3;
      break;
    }
GOMP_barrier ();
```

## 动态库函数分析

事实上在函数 GOMP_sections_start 和函数 GOMP_sections_next 当中调用的都是我们之前分析过的函数 gomp_iter_dynamic_next ，这个函数实际上就是让线程始终原子指令去竞争数据块（chunk），这个特点和 sections 需要完成的语意是相同的，只不过 sections 的块大小（chunk size）都是等于 1 的，因为一个线程一次只能够执行一个 section 代码块。

```c
unsigned
GOMP_sections_start (unsigned count)
{
  // 参数 count 的含义就是表示一共有多少个 section 代码块
  // 得到当线程的相关数据
  struct gomp_thread *thr = gomp_thread ();
  long s, e, ret;
  // 进行数据的初始化操作
  // 将数据的 chunk size 设置等于 1
  // 分割 chunk size 的起始位置设置成 1 因为根据上面的代码分析 0 表示退出循环 因此不能够使用 0 作为分割的起始位置
  if (gomp_work_share_start (false))
    {
    // 这里传入 count 作为参数的原因是需要设置 chunk 分配的最终位置 具体的源代码在下方
      gomp_sections_init (thr->ts.work_share, count);
      gomp_work_share_init_done ();
    }
  // 如果获取到一个 section 的执行权 gomp_iter_dynamic_next 返回 true 否则返回 false 
  // s 和 e 分别表示 chunk 的起始位置和终止位置 但是在 sections 当中需要注意的是所有的 chunk size 都等于 1
  // 这也很容易理解一次执行一个 section 代码块
  if (gomp_iter_dynamic_next (&s, &e))
    ret = s;
  else
    ret = 0;
  return ret;
}

// 下面是部分 gomp_sections_init 的代码
static inline void
gomp_sections_init (struct gomp_work_share *ws, unsigned count)
{
  ws->sched = GFS_DYNAMIC;
  ws->chunk_size = 1; // 设置 chunk size 等于 1
  ws->end = count + 1L; // 因为一共有 count 个 section 块
  ws->incr = 1; // 每次增长一个
  ws->next = 1; // 从 1 开始进行 chunk size 的分配 因为 0 表示退出循环（编译器角度分析）
}

unsigned
GOMP_sections_next (void)
{
  // 这个函数就比较容易理解了 就是获取一个 chunk 拿到对应的 section 的执行权
  long s, e, ret;
  if (gomp_iter_dynamic_next (&s, &e))
    ret = s;
  else
    ret = 0;
  return ret;
}

// 下面的函数在之前的很多文章当中都分析过了 这里不再进行分析
// 下面的函数的主要过程就是使用 CAS 指令不断的进行尝试，直到获取成功或者全部获取完成 没有 chunk 需要分配
bool
gomp_iter_dynamic_next (long *pstart, long *pend)
{
  struct gomp_thread *thr = gomp_thread ();
  struct gomp_work_share *ws = thr->ts.work_share;
  long start, end, nend, chunk, incr;

  end = ws->end;
  incr = ws->incr;
  chunk = ws->chunk_size;

  if (__builtin_expect (ws->mode, 1))
    {
      long tmp = __sync_fetch_and_add (&ws->next, chunk);
      if (incr > 0)
	{
	  if (tmp >= end)
	    return false;
	  nend = tmp + chunk;
	  if (nend > end)
	    nend = end;
	  *pstart = tmp;
	  *pend = nend;
	  return true;
	}
      else
	{
	  if (tmp <= end)
	    return false;
	  nend = tmp + chunk;
	  if (nend < end)
	    nend = end;
	  *pstart = tmp;
	  *pend = nend;
	  return true;
	}
    }

  start = ws->next;
  while (1)
    {
      long left = end - start;
      long tmp;

      if (start == end)
	return false;

      if (incr < 0)
	{
	  if (chunk < left)
	    chunk = left;
	}
      else
	{
	  if (chunk > left)
	    chunk = left;
	}
      nend = start + chunk;

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

## 总结

在本篇文章当中主要介绍了 OpenMP 当中 sections 的实现原理和相关的动态库函数分析，关于 sections 重点在编译器会如何对 sections 的编译指导语句进行处理的，动态库函数和 for 循环的动态调度方式是一样的，只不过 chunk size 设置成 1，分块的起始位置等于 1，分块的最终值是 section 代码块的个数，最终在动态调度的方式使用 CAS 不断获取 section 的执行权，直到所有的 section 被执行完成。

---

更多精彩内容合集可访问项目：<https://github.com/Chang-LeHung/CSCore>

关注公众号：一无是处的研究僧，了解更多计算机（Java、Python、计算机系统基础、算法与数据结构）知识。


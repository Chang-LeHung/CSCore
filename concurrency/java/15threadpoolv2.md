# 彻底了解线程池原理——动手写一个低配版的ThreadPoolExecutor

## 前言

在上篇文章当中我们介绍了一个我们自己实现的一个简易的线程池，不过这个线程值只能够支持没有返回值的线程池，而且只能够是固定线程个数的线程，而在本篇文章当中我们要去写一个与`ThreadPoolExecutor`使用方式非常相似的线程值，并且能够弥补我们上一个线程池没有的功能。

## 需求分析

在正式实现我们自己的ThreadPoolExecutor之前我们先来看一下ThreadPoolExecutor给我们提供了一些什么功能。ThreadPoolExecutor实现了一个线程池，我们先来看一下他的参数，通过他的参数分析ThreadPoolExecutor给我们提供的大致的功能，我们通过它的构造函数进行分析！

```java
public ThreadPoolExecutor(int corePoolSize,
                          int maximumPoolSize,
                          long keepAliveTime,
                          TimeUnit unit,
                          BlockingQueue<Runnable> workQueue,
                          ThreadFactory threadFactory,
                          RejectedExecutionHandler handler) {
  if (corePoolSize < 0 ||
      maximumPoolSize <= 0 ||
      maximumPoolSize < corePoolSize ||
      keepAliveTime < 0)
    throw new IllegalArgumentException();
  if (workQueue == null || threadFactory == null || handler == null)
    throw new NullPointerException();
  this.acc = System.getSecurityManager() == null ?
    null :
  AccessController.getContext();
  this.corePoolSize = corePoolSize;
  this.maximumPoolSize = maximumPoolSize;
  this.workQueue = workQueue;
  this.keepAliveTime = unit.toNanos(keepAliveTime);
  this.threadFactory = threadFactory;
  this.handler = handler;
}

```

## 自己动手实现线程池



## 总结


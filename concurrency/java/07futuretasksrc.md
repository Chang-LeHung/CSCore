# FutureTask源码深度剖析

## 前言

在前面的文章[自己动手写FutureTask](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486245&idx=1&sn=b75ded67ef8ca328f23ca2acc35dc7a8&chksm=cf0c972cf87b1e3a4cb93e707c4574cfeabab13809f72878f946b6b73439f384f2752d98a183&token=302443384&lang=zh_CN#rd)当中我们已经仔细分析了FutureTask给我们提供的功能，并且深入分析了我们改如何实现它的功能，并且给出了使用`ReentrantLock`和条件变量实现FutureTask的具体代码。而在本篇文章当中我们将仔细介绍JDK内部是如何实现FutureTask的。

## 工具准备

在JDK的`FutureTask`当中会使用到一个工具`LockSupport`，在正式介绍`FutureTask`之前我们先熟悉一下这个工具。

`LockSupport`主要是用于阻塞和唤醒线程的，它主要是通过包装`UnSafe`类，通过`UnSafe`类当中的方法进行实现的，他底层的方法是通过依赖JVM实现的。
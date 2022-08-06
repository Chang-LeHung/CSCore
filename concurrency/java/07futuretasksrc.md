# FutureTask源码深度剖析

## 前言

在前面的文章[自己动手写FutureTask](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486245&idx=1&sn=b75ded67ef8ca328f23ca2acc35dc7a8&chksm=cf0c972cf87b1e3a4cb93e707c4574cfeabab13809f72878f946b6b73439f384f2752d98a183&token=302443384&lang=zh_CN#rd)当中我们已经仔细分析了FutureTask给我们提供的功能，并且深入分析了我们改如何实现它的功能，并且给出了使用`ReentrantLock`和条件变量实现FutureTask的具体代码。而在本篇文章当中我们将仔细介绍JDK内部是如何实现FutureTask的。

## 工具准备

在JDK的`FutureTask`当中会使用到一个工具`LockSupport`，在正式介绍`FutureTask`之前我们先熟悉一下这个工具。

`LockSupport`主要是用于阻塞和唤醒线程的，它主要是通过包装`UnSafe`类，通过`UnSafe`类当中的方法进行实现的，他底层的方法是通过依赖JVM实现的。在`LockSupport`当中主要有以下两个方法：

- `unpark(Thread thread))`方法，这个方法可以给线程`thread`发放一个许可证，你可以通过多次调用这个方法给线程发放许可证，每次调用都会给线程发放一个许可证，但是这个许可证不能够进行累计，也就是说一个线程能够拥有的最大的许可证的个数是1一个。
- `park()`方法，这个线程会消费调用这个方法的线程一个许可证，因为线程的默认许可证的个数是0，如果调用一次那么许可证的数目就变成-1，当许可证的数目小于0的时候线程就会阻塞，因此如果线程从来没用调用`unpark`方法的话，那么在调用这个方法的时候会阻塞，如果线程在调用`park`方法之前，有线程调用`unpark(thread)`方法，给这个线程发放一个许可证的话，那么调用`park`方法就不会阻塞。


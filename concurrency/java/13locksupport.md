# 从零自己动手写Lock Support

## 前言

在JDK当中给我们提供的各种并发工具当中，比如`ReentrantLock`等等工具的内部实现，经常会使用到一个工具，这个工具就是`LockSupport`。

`LockSupport`给我们提供了一个非常强大的功能，他可以将一个线程阻塞也可以将一个线程唤醒。

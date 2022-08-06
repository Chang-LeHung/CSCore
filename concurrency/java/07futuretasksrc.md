# FutureTask源码深度剖析

## 前言

在前面的文章[自己动手写FutureTask](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486245&idx=1&sn=b75ded67ef8ca328f23ca2acc35dc7a8&chksm=cf0c972cf87b1e3a4cb93e707c4574cfeabab13809f72878f946b6b73439f384f2752d98a183&token=302443384&lang=zh_CN#rd)当中我们已经仔细分析了FutureTask给我们提供的功能，并且深入分析了我们改如何实现它的功能，并且给出了使用`ReentrantLock`和条件变量实现FutureTask的具体代码。而在本篇文章当中我们将仔细介绍JDK内部是如何实现FutureTask的。

## 工具准备

在JDK的`FutureTask`当中会使用到一个工具`LockSupport`，在正式介绍`FutureTask`之前我们先熟悉一下这个工具。

`LockSupport`主要是用于阻塞和唤醒线程的，它主要是通过包装`UnSafe`类，通过`UnSafe`类当中的方法进行实现的，他底层的方法是通过依赖JVM实现的。在`LockSupport`当中主要有以下三个方法：

- `unpark(Thread thread))`方法，这个方法可以给线程`thread`发放一个**许可证**，你可以通过多次调用这个方法给线程发放**许可证**，每次调用都会给线程发放一个**许可证**，但是这个**许可证**不能够进行累计，也就是说一个线程能够拥有的最大的许可证的个数是1一个。
- `park()`方法，这个线程会消费调用这个方法的线程一个许可证，因为线程的默认许可证的个数是0，如果调用一次那么许可证的数目就变成-1，当许可证的数目小于0的时候线程就会阻塞，因此如果线程从来没用调用`unpark`方法的话，那么在调用这个方法的时候会阻塞，如果线程在调用`park`方法之前，有线程调用`unpark(thread)`方法，给这个线程发放一个许可证的话，那么调用`park`方法就不会阻塞。

- `parkNanos(long nanos)`方法，同park方法一样，nanos表示最长阻塞超时时间，超时后park方法将自动返回，如果调用这个方法的线程有许可证的话也不会阻塞。

```java
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.LockSupport;

public class Demo {

  public static void main(String[] args) throws InterruptedException {
    Thread thread = new Thread(() -> {
      LockSupport.park(); // 没有许可证 阻塞住这个线程
      try {
        TimeUnit.SECONDS.sleep(1);
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
      System.out.println("阻塞完成");
    });
    thread.start();
    TimeUnit.SECONDS.sleep(2);
    LockSupport.unpark(thread); //给线程 thread 发放一个许可证
    System.out.println("线程启动");

  }
}

```

上面代码的执行结果

```java
线程启动
阻塞完成
```

从上面代码我们可以知道`LockSupport.park()`可以阻塞一个线程，因为如果没有阻塞的话肯定会先打印`阻塞完成`，因为打印这句话的线程只休眠一秒，主线程休眠两秒。

## 深入FutureTask内部

首先我们先了解一下FutureTask的几种状态：

- NEW，刚刚新建一个FutureTask对象。
- COMPLETING，FutureTask正在执行。
- NORMAL，FutureTask正常结束。
- EXCEPTIONAL，如果FutureTask对象在执行`Callable`实现类对象的`call`方法的时候出现的异常，那么FutureTask的状态就变成这个状态了。
- CANCELLED，表示FutureTask的执行过程被取消了。
- INTERRUPTING，表示正在终止FutureTask对象的执行过程。
- INTERRUPTED，表示FutureTask对象在执行的过程当中被中断了。

这些状态之间的可能的转移情况如下所示：

- NEW -> COMPLETING -> NORMAL。
- NEW -> COMPLETING -> EXCEPTIONAL。
- NEW -> CANCELLED。
- NEW -> INTERRUPTING -> INTERRUPTED。

在`FutureTask`当中用数字去表示这几个状态：

```java
private volatile int state;
private static final int NEW          = 0;
private static final int COMPLETING   = 1;
private static final int NORMAL       = 2;
private static final int EXCEPTIONAL  = 3;
private static final int CANCELLED    = 4;
private static final int INTERRUPTING = 5;
private static final int INTERRUPTED  = 6;
```






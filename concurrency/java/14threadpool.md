# 彻底了解线程池的原理——从零开始自己写线程池

## 前言

在我们的日常的编程当中，并发是始终离不开的主题，而在并发多线程当中，线程池又是一个不可规避的问题。多线程可以提高我们并发程序的效率，可以让我们不去频繁的申请和释放线程，这是一个很大的花销，而在线程池当中就不需要去频繁的申请线程，他的主要原理是申请完线程之后并不中断，而是不断的去线程池当中领取任务，然后执行，反复这样的操作。

## 线程池给我们提供的功能

我们首先来看一个使用线程池的例子：

```java
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Demo01 {

  public static void main(String[] args) {
    ExecutorService pool = Executors.newFixedThreadPool(5);
    for (int i = 0; i < 100; i++) {
      pool.execute(new Runnable() {
        @Override
        public void run() {
          for (int i = 0; i < 100; i++) {
            System.out.println(Thread.currentThread().getName() + " print " + i);
          }
        }
      });
    }
  }
}

```

在上面的例子当中，我们使用`Executors.newFixedThreadPool`去生成来一个固定线程数目的线程池，在上面的代码当中我们是使用5个线程，然后通过`execute`方法不断的去向线程池当中提交任务

<img src="../../images/concurrency/54.png" alt="54" style="zoom:80%;" />
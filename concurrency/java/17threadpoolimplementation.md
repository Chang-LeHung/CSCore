# 自己动手写乞丐版线程池

## 前言

在上篇文章[线程池的前世今生](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486999&idx=1&sn=53cc8c0a3bbf78bd35eb9f0d1322f7d6&chksm=cf0c921ef87b1b088d9db6e2d5c2de1288dc582ee181b80bf9d6a920f4f655c986ab771fb002&token=1326758682&lang=zh_CN#rd)当中我们介绍了实现线程池的原理，在这篇文章当中我们主要介绍实现一个非常简易版的线程池，深入的去理解其中的原理，麻雀虽小，五脏俱全。

## 线程池的具体实现

### 线程池实现思路

在上一篇文章[线程池的前世今生](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486999&idx=1&sn=53cc8c0a3bbf78bd35eb9f0d1322f7d6&chksm=cf0c921ef87b1b088d9db6e2d5c2de1288dc582ee181b80bf9d6a920f4f655c986ab771fb002&token=1326758682&lang=zh_CN#rd)当中我们具体去介绍了线程池当中的原理。在线程池当中我们有很多个线程不断的从任务池（用户在使用线程池的时候不断的使用`execute`方法将任务添加到线程池当中）里面去拿任务然后执行，现在需要思考我们应该用什么去实现任务池呢？

答案是阻塞队列，因为我们需要保证在多个线程往任务池里面加入任务的时候并发安全，JDK已经给我们提供了这样的数据结构——`BlockingQueue`，这个是一个并发安全的阻塞队列，他之所以叫做阻塞队列，是因为我们可以设置队列当中可以容纳数据的个数，当加入到队列当中的数据超过这个值的时候，试图将数据加入到阻塞队列当中的线程就会被挂起。当队列当中为空的时候，试图从队列当中取出数据的线程也会被挂起。

### Worker实现代码

```java
package cscore.concurrent.java.threadpool;

import java.util.concurrent.BlockingQueue;

public class Worker implements Runnable {

  private Thread thisThread;
  private BlockingQueue<Runnable> taskQueue;
  private volatile boolean isStopped;

  public Worker(BlockingQueue taskQueue) {
    this.taskQueue = taskQueue;
  }

  @Override
  public void run() {
    thisThread = Thread.currentThread();
    while (!isStopped) {
      try {
        Runnable task = taskQueue.take();
        task.run();
      } catch (InterruptedException e) {
        // do nothing
      }
    }
  }

  public synchronized void stopWorker() {
    if (isStopped) {
      throw new RuntimeException("thread has been interrupted");
    }
    isStopped = true;
    thisThread.interrupt();
  }

  public synchronized boolean isStopped() {
    return isStopped;
  }

  public static void main(String[] args) {
    System.out.println(Integer.toBinaryString(~0xf));
  }
}

```

### 线程池实现代码

```java
package cscore.concurrent.java.threadpool;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

public class ThreadPool {

  private BlockingQueue taskQueue;
  private volatile boolean isStopped;
  private final List<Worker> workers = new ArrayList<>();

  public ThreadPool(int numThreads, int maxTasks) {
    this.taskQueue = new ArrayBlockingQueue(maxTasks);
    for (int i = 0; i < numThreads; i++) {
      workers.add(new Worker(this.taskQueue));
    }
    int i = 1;
    for (Worker worker : workers) {
      new Thread(worker, "ThreadPool-" + i + "-thread").start();
      i++;
    }
  }

  public void execute(Runnable runnable) throws InterruptedException {
    if (isStopped) {
      System.err.println("thread pool has been stopped, so quit submitting task");
      return;
    }
    taskQueue.put(runnable);
  }

  public synchronized void stop() {
    if (isStopped)
      return;
    isStopped = true;
    for (Worker worker : workers) {
      worker.stopWorker();
    }
  }

  public synchronized void shutDown() {
    waitForAllTasks();
    stop();
  }

  private void waitForAllTasks() {
    while (taskQueue.size() > 0)
      Thread.yield();
  }
}
```

### 线程池测试代码

```java
package cscore.concurrent.java.threadpool;

public class TestPool {

  public static void main(String[] args) throws InterruptedException {
    ThreadPool pool = new ThreadPool(10, 1024);

    for (int i = 0; i < 1000; i++) {
      pool.execute(() -> {
        System.out.println(Thread.currentThread().getName() + " say hello");
      });
    }
    pool.shutDown();
  }
}
```


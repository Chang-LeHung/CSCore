# 自己动手写线程池

## 前言

在上篇文章[线程池的前世今生](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486999&idx=1&sn=53cc8c0a3bbf78bd35eb9f0d1322f7d6&chksm=cf0c921ef87b1b088d9db6e2d5c2de1288dc582ee181b80bf9d6a920f4f655c986ab771fb002&token=1326758682&lang=zh_CN#rd)当中我们介绍了实现线程池的原理，在这篇文章当中我们主要介绍实现一个非常简易版的线程池，深入的去理解其中的原理。

## 线程池的具体实现

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


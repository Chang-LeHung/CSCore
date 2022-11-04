# 自己动手写乞丐版线程池

## 前言

在上篇文章[线程池的前世今生](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486999&idx=1&sn=53cc8c0a3bbf78bd35eb9f0d1322f7d6&chksm=cf0c921ef87b1b088d9db6e2d5c2de1288dc582ee181b80bf9d6a920f4f655c986ab771fb002&token=1326758682&lang=zh_CN#rd)当中我们介绍了实现线程池的原理，在这篇文章当中我们主要介绍实现一个非常简易版的线程池，深入的去理解其中的原理，麻雀虽小，五脏俱全。

## 线程池的具体实现

### 线程池实现思路

#### 任务保存到哪里？

在上一篇文章[线程池的前世今生](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486999&idx=1&sn=53cc8c0a3bbf78bd35eb9f0d1322f7d6&chksm=cf0c921ef87b1b088d9db6e2d5c2de1288dc582ee181b80bf9d6a920f4f655c986ab771fb002&token=1326758682&lang=zh_CN#rd)当中我们具体去介绍了线程池当中的原理。在线程池当中我们有很多个线程不断的从任务池（用户在使用线程池的时候不断的使用`execute`方法将任务添加到线程池当中）里面去拿任务然后执行，现在需要思考我们应该用什么去实现任务池呢？

答案是阻塞队列，因为我们需要保证在多个线程往任务池里面加入任务的时候并发安全，JDK已经给我们提供了这样的数据结构——`BlockingQueue`，这个是一个并发安全的阻塞队列，他之所以叫做阻塞队列，是因为我们可以设置队列当中可以容纳数据的个数，当加入到队列当中的数据超过这个值的时候，试图将数据加入到阻塞队列当中的线程就会被挂起。当队列当中为空的时候，试图从队列当中取出数据的线程也会被挂起。

#### 线程的设计

在我们自己实现的线程池当中我们定一个`Worker`类去不断的从任务池当中取出任务，然后进行执行。在我们自己定义的`worker`当中还需要有一个变量`isStopped`表示线程是否停止工作。同时在`worker`当中还需要保存当前是哪个线程在执行任务，因此在我们自己设计的`woker`类当中还需要有一个`thisThread`变量，保存正在执行任务的线程，因此`worker`的整体设计如下：

```java
package cscore.concurrent.java.threadpool;

import java.util.concurrent.BlockingQueue;

public class Worker implements Runnable {

  private Thread thisThread; // 表示正在执行任务的线程
  private BlockingQueue<Runnable> taskQueue; // 由线程池传递过来的任务队列
  private volatile boolean isStopped; // 表示 worker 是否停止工作 需要使用 volatile 保证线程之间的可见性

  public Worker(BlockingQueue taskQueue) { // 这个构造方法是在线程池的实现当中会被调用
    this.taskQueue = taskQueue;
  }

  // 线程执行的函数
  @Override
  public void run() {
    thisThread = Thread.currentThread(); // 获取执行任务的线程
    while (!isStopped) { // 当线程没有停止的时候就不断的去任务池当中取出任务
      try {
        Runnable task = taskQueue.take(); // 从任务池当中取出任务 当没有任务的时候线程会被这个方法阻塞
        task.run(); // 执行任务 任务就是一个 Runnable 对象
      } catch (InterruptedException e) {
        // do nothing
        // 这个地方很重要 你有没有思考过一个问题当任务池当中没有任务的时候 线程会被阻塞在 take 方法上
        // 如果我们后面没有任务提交拿他就会一直阻塞 那么我们该如何唤醒他呢
        // 答案就在下面的函数当中 调用线程的 interruput 方法 那么take方法就会产生一个异常 然后我们
        // 捕获到一异常 然后线程退出
      }
    }
  }

  public synchronized void stopWorker() {
    if (isStopped) {
      throw new RuntimeException("thread has been interrupted");
    }
    isStopped = true;
    thisThread.interrupt(); // 中断线程产生异常
  }

  public synchronized boolean isStopped() {
    return isStopped;
  }
}

```

#### 线程池的参数

在我们自己实现的线程池当中，我们只需要定义两个参数一个是线程的个数，另外一个是阻塞队列（任务池）当中最大的任务个数。在我们自己实现的线程池当中还需要有一个变量`isStopped`表示线程池是否停止工作了，因此线程池的初步设计大致如下：

```java
  private BlockingQueue taskQueue; // 任务池
  private volatile boolean isStopped; // 
  private final List<Worker> workers = new ArrayList<>();// 保存所所有的执行任务的线程

  public ThreadPool(int numThreads, int maxTasks) {
    this.taskQueue = new ArrayBlockingQueue(maxTasks);
    for (int i = 0; i < numThreads; i++) {
      workers.add(new Worker(this.taskQueue));
    }
    int i = 1;
    // 这里产生线程 然后启动线程
    for (Worker worker : workers) {
      new Thread(worker, "ThreadPool-" + i + "-thread").start();
      i++;
    }
  }
```

### 线程池实现代码

在上文当中我们大致设计的线程池的初步结构，从上面的结果可以看出当我们造一个`ThreadPool`对象的时候会产生指定线程的数目线程并且启动他们去执行任务，现在我们还需要设计的就是如何关闭线程！我们在关闭线程的时候还需要保证所有的任务都被执行完成然后才关闭所有的线程，再退出，我们设计这个方法为`shutDown`。除此之外我们还设计一个函数可以强制退出，不用执行所有的任务了，就直接退出，这个方法为`stop`。整个线程池实现的代码如下：

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

  // 下面这个方法是向线程池提交任务
  public void execute(Runnable runnable) throws InterruptedException {
    if (isStopped) {
      // 如果线程池已经停下来了，就不在向任务队列当中提交任务了
      System.err.println("thread pool has been stopped, so quit submitting task");
      return;
    }
    taskQueue.put(runnable);
  }

  // 强制关闭线程池
  public synchronized void stop() {
    isStopped = true;
    for (Worker worker : workers) {
      worker.stopWorker();
    }
  }

  public synchronized void shutDown() {
    // 先表示关闭线程池 线程就不能再向线程池提交任务
    isStopped = true;
    // 先等待所有的任务执行完成再关闭线程池
    waitForAllTasks();
    stop();
  }

  private void waitForAllTasks() {
    // 当线程池当中还有任务的时候 就不退出循环
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
    ThreadPool pool = new ThreadPool(3, 1024);

    for (int i = 0; i < 10; i++) {
      int tmp = i;
      pool.execute(() -> {
        System.out.println(Thread.currentThread().getName() + " say hello " + tmp);
      });
    }
    pool.shutDown();
  }
}
```

上面的代码输出结果：

```java
ThreadPool-2-thread say hello 1
ThreadPool-2-thread say hello 3
ThreadPool-2-thread say hello 4
ThreadPool-2-thread say hello 5
ThreadPool-2-thread say hello 6
ThreadPool-2-thread say hello 7
ThreadPool-2-thread say hello 8
ThreadPool-2-thread say hello 9
ThreadPool-3-thread say hello 2
ThreadPool-1-thread say hello 0
```

从上面的结果来看确实实现了线程池的效果。

### 杂谈

可能你会有疑问，当我们调用 `interrupt`的时候是如何产生异常的，我们仔细看一个阻塞队列的实现。在`ArrayBlockingQueue`当中`take`方法实现如下：

```java
    public E take() throws InterruptedException {
        final ReentrantLock lock = this.lock;
        lock.lockInterruptibly();
        try {
            while (count == 0)
                notEmpty.await();
            return dequeue();
        } finally {
            lock.unlock();
        }
    }
```

在这个方法当中调用的是锁的`lock.lockInterruptibly();`方法，当调用这个方法的时候线程是可以被`interrupt`方法中断的，然后会抛出`InterruptedException`异常。

## 总结

在本篇文章当中我们主要实现了一个乞丐版的线程池，这个线程池离JDK给我们提供的线程池还是有一点距离，JDK给我们提供给的线程池还有很多其他的参数，我们将在后续的几篇文章当中继续向JDK给我们提供的线程池靠近，直至实现一个盗版的JDK的线程池。本篇文章的代码在下面的链接当中也可以访问。

---

以上就是本篇文章的所有内容了，我是**LeHung**，我们下期再见！！！更多精彩内容合集可访问项目：<https://github.com/Chang-LeHung/CSCore>

关注公众号：**一无是处的研究僧**，了解更多计算机（Java、Python、计算机系统基础、算法与数据结构）知识。

![](https://img2022.cnblogs.com/blog/2519003/202207/2519003-20220703200459566-1837431658.jpg)


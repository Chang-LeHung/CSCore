# 自己动手写线程池——向JDK线程池进发

## 前言

在前面的文章[自己动手写乞丐版线程池](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247487063&idx=1&sn=89f3025d3b23e399ced13e6991e8afb2&chksm=cf0c925ef87b1b480165986028d0605672d6047b9fd4111831a70f2b9b5dc5e6e01902e60efb&token=624247549&lang=zh_CN#rd)中，我们写了一个非常简单的线程池实现，这个只是一个非常简单的实现，在本篇文章当中我们将要实现一个和JDK内部实现的线程池非常相似的线程池。

## JDK线程池一瞥

我们首先看一个JDK给我们提供的线程池`ThreadPoolExecutor`的构造函数的参数：

```java
public ThreadPoolExecutor(int corePoolSize,
                              int maximumPoolSize,
                              long keepAliveTime,
                              TimeUnit unit,
                              BlockingQueue<Runnable> workQueue,
                              ThreadFactory threadFactory,
                              RejectedExecutionHandler handler) 
```

参数解释：

- corePoolSize：这个参数你可以理解为线程池当中至少需要 corePoolSize 个线程，初始时线程池当中线程的个数为0，当线程池当中线程的个数小于 corePoolSize 每次提交一个任务都会创建一个线程，并且先执行这个提交的任务，然后再去任务队列里面去获取新的任务，然后再执行。
- maximumPoolSize：这个参数指的是线程池当中能够允许的最大的线程的数目，当任务队列满了之后如果这个时候有新的任务想要加入队列当中，当发现队列满了之后就创建新的线程去执行任务，但是需要满足最大的线程的个数不能够超过 maximumPoolSize 。
- keepAliveTime 和 unit：这个主要是用于时间的表示，当队列当中多长时间没有数据的时候线程自己退出，前面谈到了线程池当中任务过多的时候会超过 corePoolSize ，当线程池闲下来的时候这些多余的线程就可以退出了。
- workQueue：这个就是用于保存任务的阻塞队列。
- threadFactory：这个参数倒不是很重要，线程工厂。
- handler：这个表示拒绝策略，JDK给我们提供了四种策略：
  - AbortPolicy：抛出异常。
  - DiscardPolicy：放弃这个任务。
  - CallerRunPolicy：提交任务的线程执行。
  - DiscardOldestPolicy:放弃等待时间最长的任务。

如果上面的参数你不能够理解，可以先阅读这篇文章[自己动手写乞丐版线程池](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247487063&idx=1&sn=89f3025d3b23e399ced13e6991e8afb2&chksm=cf0c925ef87b1b480165986028d0605672d6047b9fd4111831a70f2b9b5dc5e6e01902e60efb&token=624247549&lang=zh_CN#rd)。

## 线程池实现

根据前面的参数分析我们自己实现的线程池需要实现一下功能：

- 能够提交Runnable的任务和Callable的任务。
- 线程池能够自己实现动态的扩容和所容，动态调整线程池当中线程的数目，当任务多的时候能够增加线程的数目，当任务少的时候多出来的线程能够自动退出。
- 有自己的拒绝策略，当任务队列满了，线程数也达到最大的时候，需要拒绝提交的任务。

### 实现Runnable

```java
  // 下面这个方法是向线程池提交任务
  public void execute(Runnable runnable) throws InterruptedException {
    checkPoolState();

    if (addWorker(runnable, false)  // 如果能够加入新的线程执行任务 加入成功就直接返回
            || !taskQueue.offer(runnable) // 如果 taskQueue.offer(runnable) 返回 false 说明提交任务失败 任务队列已经满了
            || addWorker(runnable, true)) // 使用能够使用的最大的线程数 (maximumPoolSize) 看是否能够产生新的线程
      return;

    // 如果任务队列满了而且不能够加入新的线程 则拒绝这个任务
    if (!taskQueue.offer(runnable))
      reject(runnable);
  }

```

在上面的代码当中：

- checkPoolState函数是检查线程池的状态，当线程池被停下来之后就不能够在提交任务：

```java
  private void checkPoolState() {
    if (isStopped) {
      // 如果线程池已经停下来了，就不在向任务队列当中提交任务了
      throw new RuntimeException("thread pool has been stopped, so quit submitting task");
    }
  }

```

- addWorker函数是往线程池当中提交任务并且产生一个线程，并且这个线程执行的第一个任务就是传递的参数。max表示线程的最大数目，max == true 的时候表示使用 maximumPoolSize 否则使用 corePoolSize，当返回值等于 true 的时候表示执行成功，否则表示执行失败。

```java
  /**
   *
   * @param runnable 需要被执行的任务
   * @param max 是否使用 maximumPoolSize
   * @return boolean
   */
  public synchronized boolean addWorker(Runnable runnable, boolean max) {

    if (ct.get() >= corePoolSize && !max)
      return false;
    if (ct.get() >= maximumPoolSize && max)
      return false;
    Worker worker = new Worker(runnable);
    workers.add(worker);
    Thread thread = new Thread(worker, "ThreadPool-" + "Thread-" + ct.addAndGet(1));
    thread.start();
    return true;
  }

```

### 实现Callable

这个函数其实比较简单，只需要将传入的Callable对象封装成一个FutureTask对象即可，因为FutureTask实现了Callable和Runnable两个接口。

```java
  public <V> RunnableFuture<V> submit(Callable<V> task) throws InterruptedException {
    checkPoolState();
    FutureTask<V> futureTask = new FutureTask<>(task);
    execute(futureTask);
    return futureTask;
  }

```



```java
package cscore.concurrent.java.threadpoolv2;


import java.util.ArrayList;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

public class ThreadPool {

  private AtomicInteger ct = new AtomicInteger(0); // 当前在执行任务的线程个数
  private int corePoolSize;
  private int maximumPoolSize;
  private long keepAliveTime;
  private TimeUnit unit;
  private BlockingQueue<Runnable> taskQueue;
  private RejectPolicy policy;

  private ArrayList<Worker> workers = new ArrayList<>();

  private volatile boolean isStopped;
  private boolean useTimed;

  public int getCt() {
    return ct.get();
  }

  public ThreadPool(int corePoolSize, int maximumPoolSize, TimeUnit unit, long keepAliveTime, RejectPolicy policy
                    , int maxTasks) {
    // please add -ea to vm options to make assert keyword enable
    assert corePoolSize > 0;
    assert maximumPoolSize > 0;
    assert keepAliveTime >= 0;
    assert maxTasks > 0;

    this.corePoolSize = corePoolSize;
    this.maximumPoolSize = maximumPoolSize;
    this.unit = unit;
    this.policy = policy;
    this.keepAliveTime = keepAliveTime;
    taskQueue = new ArrayBlockingQueue<Runnable>(maxTasks);
    useTimed = keepAliveTime != 0;
  }

  /**
   *
   * @param runnable 需要被执行的任务
   * @param max 是否使用 maximumPoolSize
   * @return boolean
   */
  public synchronized boolean addWorker(Runnable runnable, boolean max) {

    if (ct.get() >= corePoolSize && !max)
      return false;
    if (ct.get() >= maximumPoolSize && max)
      return false;
    Worker worker = new Worker(runnable);
    workers.add(worker);
    Thread thread = new Thread(worker, "ThreadPool-" + "Thread-" + ct.addAndGet(1));
    thread.start();
    return true;
  }

  // 下面这个方法是向线程池提交任务
  public void execute(Runnable runnable) throws InterruptedException {
    checkPoolState();

    if (addWorker(runnable, false)  // 如果能够加入新的线程执行任务 加入成功就直接返回
            || !taskQueue.offer(runnable) // 如果 taskQueue.offer(runnable) 返回 false 说明提交任务失败 任务队列已经满了
            || addWorker(runnable, true)) // 使用能够使用的最大的线程数 (maximumPoolSize) 看是否能够产生新的线程
      return;

    // 如果任务队列满了而且不能够加入新的线程 则拒绝这个任务
    if (!taskQueue.offer(runnable))
      reject(runnable);
  }

  private void reject(Runnable runnable) throws InterruptedException {
    switch (policy) {
      case ABORT:
        throw new RuntimeException("task queue is full");
      case CALLER_RUN:
        runnable.run();
      case DISCARD:
        return;
      case DISCARD_OLDEST:
        // 放弃等待时间最长的任务
        taskQueue.poll();
        execute(runnable);
    }
  }

  private void checkPoolState() {
    if (isStopped) {
      // 如果线程池已经停下来了，就不在向任务队列当中提交任务了
      throw new RuntimeException("thread pool has been stopped, so quit submitting task");
    }
  }

  public <V> RunnableFuture<V> submit(Callable<V> task) throws InterruptedException {
    checkPoolState();
    FutureTask<V> futureTask = new FutureTask<>(task);
    execute(futureTask);
    return futureTask;
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
    while (taskQueue.size() > 0) {
      Thread.yield();
      try {
        Thread.sleep(1000);
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }
  }

   class Worker implements Runnable {

    private Thread thisThread;

    private final Runnable firstTask;
    private volatile boolean isStopped;

    public Worker(Runnable firstTask) {
      this.firstTask = firstTask;
    }

    @Override
    public void run() {
      // 先执行传递过来的第一个任务 这里是一个小的优化 让线程直接执行第一个任务 不需要
      // 放入任务队列再取出来执行了
      firstTask.run();

      thisThread = Thread.currentThread();
      while (!isStopped || taskQueue.size() != 0) {
        try {
          Runnable task = useTimed ? taskQueue.poll(keepAliveTime, unit) : taskQueue.take();
          if (task == null) {
            int i;
            boolean exit = true;
            if (ct.get() > corePoolSize) {
              do{
                i = ct.get();
                if (i <= corePoolSize) {
                  exit = false;
                  break;
                }
              }while (!ct.compareAndSet(i, i - 1));
              if (exit) {
                return;
              }
            }
          }else {
            task.run();
          }
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

  }

}
```



```java
ThreadPool-Thread-2 output a
ThreadPool-Thread-1 output a
ThreadPool-Thread-3 output a
ThreadPool-Thread-4 output a
Number Threads = 5
ThreadPool-Thread-5 output a
ThreadPool-Thread-2 output a
ThreadPool-Thread-1 output a
ThreadPool-Thread-3 output a
ThreadPool-Thread-4 output a
ThreadPool-Thread-5 output a
ThreadPool-Thread-2 output a
ThreadPool-Thread-1 output a
ThreadPool-Thread-4 output a
ThreadPool-Thread-3 output a
ThreadPool-Thread-5 output a
ThreadPool-Thread-2 output a
ThreadPool-Thread-1 output a
ThreadPool-Thread-4 output a
Number Threads = 5
Number Threads = 5
Number Threads = 5
Number Threads = 5
Number Threads = 5
Number Threads = 5
Number Threads = 5
Number Threads = 5
Number Threads = 5
Number Threads = 3
Number Threads = 2
Number Threads = 2
Number Threads = 2
Number Threads = 2
```






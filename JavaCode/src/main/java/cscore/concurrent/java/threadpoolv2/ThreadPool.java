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
      while (!isStopped) {
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
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
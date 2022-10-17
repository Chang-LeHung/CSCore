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

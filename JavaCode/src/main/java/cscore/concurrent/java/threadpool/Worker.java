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

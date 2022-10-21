package cscore.concurrent.java.threadpoolv2;

import java.util.concurrent.TimeUnit;

public class Test {

  public static void main(String[] args) throws InterruptedException {
    var pool = new ThreadPool(2, 5, TimeUnit.SECONDS, 10, RejectPolicy.ABORT, 100000);

    for (int i = 0; i < 10; i++) {
      pool.execute(() -> {
        System.out.println(Thread.currentThread().getName() + " output a");
        try {
          Thread.sleep(10);
        } catch (InterruptedException e) {
          e.printStackTrace();
        }
      });
    }
    int n = 15;
    while (n-- > 0) {
      System.out.println("Number Threads = " + pool.getCt());
      Thread.sleep(1000);
    }
    pool.shutDown();
  }
}

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

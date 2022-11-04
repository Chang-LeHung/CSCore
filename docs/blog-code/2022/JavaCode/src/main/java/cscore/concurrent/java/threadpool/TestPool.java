package cscore.concurrent.java.threadpool;

public class TestPool {

  public static void main(String[] args) throws InterruptedException {
    ThreadPool pool = new ThreadPool(3, 1024);

    for (int i = 0; i < 10; i++) {
      int tmp = i;
      pool.execute(() -> System.out.println(Thread.currentThread().getName() + " say hello " + tmp));
    }
    pool.shutDown();
  }
}

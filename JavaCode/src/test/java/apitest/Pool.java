package apitest;

import org.junit.Test;

import java.util.concurrent.*;

public class Pool {

  @Test
  public void testPool() throws InterruptedException {
    ExecutorService pool = Executors.newFixedThreadPool(5);
    ArrayBlockingQueue<Integer> integers = new ArrayBlockingQueue<>(2);
    System.out.println(integers.poll(5, TimeUnit.SECONDS));
  }
}

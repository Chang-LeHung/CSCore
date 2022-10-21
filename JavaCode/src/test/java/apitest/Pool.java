package apitest;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.concurrent.*;

public class Pool {

  @Test
  public void testPool() throws InterruptedException {
    ExecutorService pool = Executors.newFixedThreadPool(5);
    ArrayBlockingQueue<Integer> integers = new ArrayBlockingQueue<>(2);
    System.out.println(integers.poll(5, TimeUnit.SECONDS));
  }

  @Test
  public void testArray() {
    ArrayList<Integer> integers = new ArrayList<>();
    for(int i = 0; i < 10; i++) {
      integers.add(i);
    }
    Iterator<Integer> iterator = integers.iterator();
    while (iterator.hasNext()) {
      Integer next = iterator.next();
      if (next.equals(5)) {
        iterator.remove();
      }
    }
    System.out.println(integers);
  }
}

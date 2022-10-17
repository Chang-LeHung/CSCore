package apitest;

import org.junit.Test;

import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadPoolExecutor;

public class Pool {

  @Test
  public void testPool() {
    ExecutorService pool = Executors.newFixedThreadPool(5);

  }
}

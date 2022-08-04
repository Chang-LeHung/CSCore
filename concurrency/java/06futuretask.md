# 手写FutureTask以及FutureTask源码剖析

## 前言

在并发编程当中我们最常见的需求就是启动一个线程执行一个函数去完成我们的需求，而在这种需求当中，我们常常需要函数有返回值。比如我们需要同一个非常大的数组当中数据的和，让每一个线程求某一个区间内部的和，最终将这些和加起来，那么每个线程都需要返回对应区间的和。而在Java当中给我们提供了这种机制，去实现这一个效果——`FutureTask`。

## FutureTask

在自己写`FutureTask`之前我们首先写一个例子来回顾一下`FutureTask`的编程步骤：

- 写一个类实现`Callable`接口。
- `new`一个`FutureTask`对象，并且`new`一个第一步写的类，`new FutureTask<>(callable实现类)`。
- 最后将刚刚得到的`FutureTask`对象传入`Thread`类当中，然后启动线程即可。
- 然后我们可以调用`FutureTask`的`get`方法得到返回的结果。

>假如有一个数组`data`，长度为100000，现在有10个线程，第`i`个线程求数组`[i * 10000, (i + 1) * 10000)`所有数据的和，然后将这十个线程的结果加起来。

```java
import java.lang.reflect.Array;
import java.util.Arrays;
import java.util.Random;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

public class FutureTaskDemo {

  public static void main(String[] args) throws ExecutionException, InterruptedException {
    int[] data = new int[100000];
    Random random = new Random();
    for (int i = 0; i < 100000; i++) {
      data[i] = random.nextInt(10000);
    }
    @SuppressWarnings("unchecked")
    FutureTask<Integer>[] tasks = (FutureTask<Integer>[]) Array.newInstance(FutureTask.class, 10);
    // 设置10个 futuretask 任务计算数组当中数据的和
    for (int i = 0; i < 10; i++) {
      int idx = i;
      tasks[i] = new FutureTask<>(() -> {
        int sum = 0;
        for (int k = idx * 10000; k < (idx + 1) * 10000; k++) {
          sum += data[k];
        }
        return sum;
      });
    }
    // 开启线程执行 futureTask 任务
    for (FutureTask<Integer> futureTask : tasks) {
      new Thread(futureTask).start();
    }
    int threadSum = 0;
    for (FutureTask<Integer> futureTask : tasks) {
      threadSum += futureTask.get();
    }
    int sum = Arrays.stream(data).sum();
    System.out.println(sum == threadSum); // 结果始终为 true
  }
}

```


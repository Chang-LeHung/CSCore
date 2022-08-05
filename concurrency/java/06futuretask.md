# 60行自己动手写FutureTask

## 前言

在并发编程当中我们最常见的需求就是启动一个线程执行一个函数去完成我们的需求，而在这种需求当中，我们常常需要函数有返回值。比如我们需要同一个非常大的数组当中数据的和，让每一个线程求某一个区间内部的和，最终将这些和加起来，那么每个线程都需要返回对应区间的和。而在Java当中给我们提供了这种机制，去实现这一个效果——`FutureTask`。

## FutureTask

在自己写`FutureTask`之前我们首先写一个例子来回顾一下`FutureTask`的编程步骤：

- 写一个类实现`Callable`接口。

```java
@FunctionalInterface
public interface Callable<V> {
    /**
     * Computes a result, or throws an exception if unable to do so.
     *
     * @return computed result
     * @throws Exception if unable to compute a result
     */
    V call() throws Exception;
}
```

实现接口就实现`call`即可，可以看到这个函数是有返回值的，而`FutureTask`返回给我们的值就是这个函数的返回值。

- `new`一个`FutureTask`对象，并且`new`一个第一步写的类，`new FutureTask<>(callable实现类)`。
- 最后将刚刚得到的`FutureTask`对象传入`Thread`类当中，然后启动线程即可`new Thread(futureTask).start();`。
- 然后我们可以调用`FutureTask`的`get`方法得到返回的结果`futureTask.get();`。

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

可能你会对`FutureTask`的使用方式感觉困惑，或者不是很清楚，现在我们来仔细捋一下思路。

1. 首先启动一个线程要么是继承自`Thread`类，然后重写`Thread`类的`run`方法，要么是给`Thread`类传递一个实现了`Runnable`的类对象，当然可以用匿名内部类实现。
2. 既然我们的`FutureTask`对象可以传递给`Thread`类，说明`FutureTask`肯定是实现了`Runnable`接口，我们现在来看一下`FutureTask`的继承体系。

<img src="../../images/concurrency/38.png" alt="38" style="zoom:80%;" />

​	可以发现的是`FutureTask`确实实现了`Runnable`接口，同时还实现了`Future`接口，这个`Future`接口主要提供了后面我们使用`FutureTask`的一系列函数比如`get`。

3. 看到这里你应该能够大致想到在`FutureTask`中的`run`方法会调用`Callable`当中实现的`call`方法，然后将结果保存下来，当调用`get`方法的时候再将这个结果返回。

## 自己实现FutureTask

### 工具准备

经过上文的分析你可能已经大致了解了`FutureTask`的大致执行过程了，但是需要注意的是，如果你执行`FutureTask`的`get`方法是可能阻塞的，因为可能`Callable`的`call`方法还没有执行完成。因此在`get`方法当中就需要有阻塞线程的代码，但是当`call`方法执行完成之后需要将这些线程都唤醒。

在本篇文章当中使用锁`ReentrantLock`和条件变量`Condition`进行线程的阻塞和唤醒，在我们自己动手实现`FutureTask`之前，我们先熟悉一下上面两种工具的使用方法。

- `ReentrantLock`主要有两个方法：
  - `lock`对临界区代码块进行加锁。
  - `unlock`对临界区代码进行解锁。
- `Condition`主要有三个方法：
  - `await`阻塞调用这个方法的线程，等待其他线程唤醒。
  - `signal`唤醒一个被`await`方法阻塞的线程。
  - `signalAll`唤醒所有被`await`方法阻塞的线程。

```java
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

public class LockDemo {

  private ReentrantLock lock;
  private Condition condition;

  LockDemo() {
    lock = new ReentrantLock();
    condition = lock.newCondition();
  }

  public void blocking() {
    lock.lock();
    try {
      System.out.println(Thread.currentThread() + " 准备等待被其他线程唤醒");
      condition.await();
    } catch (InterruptedException e) {
      e.printStackTrace();
    }finally {
      lock.unlock();
    }
  }

  public void inform() throws InterruptedException {
    // 先休眠两秒 等他其他线程先阻塞
    TimeUnit.SECONDS.sleep(2);
    lock.lock();
    try {
      System.out.println(Thread.currentThread() + " 准备唤醒其他线程");
      condition.signal(); // 唤醒一个被 await 方法阻塞的线程
      // condition.signalAll(); // 唤醒所有被 await 方法阻塞的线程
    }finally {
      lock.unlock();
    }
  }

  public static void main(String[] args) {
    LockDemo lockDemo = new LockDemo();
    Thread thread = new Thread(() -> {
      lockDemo.blocking(); // 执行阻塞线程的代码
    }, "Blocking-Thread");
    Thread thread1 = new Thread(() -> {
      try {
        lockDemo.inform(); // 执行唤醒线程的代码
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }, "Inform-Thread");
    thread.start();
    thread1.start();
  }
}
```

上面的代码的输出：

```
Thread[Blocking-Thread,5,main] 准备等待被其他线程唤醒
Thread[Inform-Thread,5,main] 准备唤醒其他线程
```

### FutureTask设计与实现

在前文当中我们已经谈到了`FutureTask`的实现原理，主要有以下几点：

- 构造函数需要传入一个实现了`Callable`接口的类对象，这个将会在`FutureTask`的`run`方法执行，然后得到函数的返回值，并且将返回值存储起来。
- 当线程调用`get`方法的时候，如果这个时候`Callable`当中的`call`已经执行完成，直接返回`call`函数返回的结果就行，如果`call`函数还没有执行完成，那么就需要将调用`get`方法的线程挂起，这里我们可以使用`condition.await()`将线程挂起。
- 在`call`函数执行完成之后，需要将之前被`get`方法挂起的线程唤醒继续执行，这里使用`condition.signalAll()`将所有挂起的线程唤醒。
- 因为是我们自己实现`FutureTask`，功能不会那么齐全，只需要能够满足我们的主要需求即可，主要是帮助大家了解`FutureTask`原理。

实现代码如下（分析都在注释当中）：

```java
import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

// 这里需要实现 Runnable 接口，因为需要将这个对象放入 Thread 类当中
// 而 Thread 要求传入的对象实现了 Runnable 接口
public class MyFutureTask<V> implements Runnable {

  private final Callable<V> callable;
  private Object returnVal; // 这个表示我们最终的返回值
  private final ReentrantLock lock;
  private final Condition condition;

  public MyFutureTask(Callable<V> callable) {
    // 将传入的 callable 对象存储起来 方便在后面的 run 方法当中调用
    this.callable = callable;
    lock = new ReentrantLock();
    condition = lock.newCondition();
  }

  @SuppressWarnings("unchecked")
  public V get(long timeout, TimeUnit unit) {
    if (returnVal != null) // 如果符合条件 说明 call 函数已经执行完成 返回值已经不为 null 了
      return (V) returnVal; // 直接将结果返回即可 这样不用竞争锁资源 提高程序执行效率
    lock.lock();
    try {
      // 这里需要进行二次判断 (双重检查)
      // 因为如果一个线程在第一次判断 returnVal 为空
      // 然后这个时候它可能因为获取锁而被挂起
      // 而在被挂起的这段时间，call 可能已经执行完成
      // 如果这个时候不进行判断直接执行 await方法
      // 那后面这个线程将无法被唤醒
      if (returnVal == null)
        condition.await(timeout, unit);
    } catch (InterruptedException e) {
      e.printStackTrace();
    } finally {
      lock.unlock();
    }
    return (V) returnVal;
  }

  @SuppressWarnings("unchecked")
  public V get() {
    if (returnVal != null)
      return (V) returnVal;
    lock.lock();
    try {
      // 同样的需要进行双重检查
      if (returnVal == null)
      	condition.await();
    } catch (InterruptedException e) {
      e.printStackTrace();
    } finally {
      lock.unlock();
    }
    return (V) returnVal;
  }


  @Override
  public void run() {
    if (returnVal != null)
      return;
    try {
      // 在 Runnable 的 run 方法当中
      // 执行 Callable 方法的 call 得到返回结果
      returnVal = callable.call();
    } catch (Exception e) {
      e.printStackTrace();
    }
    lock.lock();
    try {
      // 因为已经得到了结果
      // 因此需要将所有被 await 方法阻塞的线程唤醒
      // 让他们从 get 方法返回
      condition.signalAll();
    }finally {
      lock.unlock();
    }
  }
	// 下面是测试代码
  public static void main(String[] args) {
    MyFutureTask<Integer> ft = new MyFutureTask<>(() -> {
      TimeUnit.SECONDS.sleep(2);
      return 101;
    });
    Thread thread = new Thread(ft);
    thread.start();
    System.out.println(ft.get(100, TimeUnit.MILLISECONDS)); // 输出为 null
    System.out.println(ft.get()); // 输出为 101
  }
}

```

我们现在用我们自己写的`MyFutureTask`去实现在前文当中数组求和的例子：

```java
public static void main(String[] args) throws ExecutionException, InterruptedException {
  int[] data = new int[100000];
  Random random = new Random();
  for (int i = 0; i < 100000; i++) {
    data[i] = random.nextInt(10000);
  }
  @SuppressWarnings("unchecked")
  MyFutureTask<Integer>[] tasks = (MyFutureTask<Integer>[]) Array.newInstance(MyFutureTask.class, 10);
  for (int i = 0; i < 10; i++) {
    int idx = i;
    tasks[i] = new MyFutureTask<>(() -> {
      int sum = 0;
      for (int k = idx * 10000; k < (idx + 1) * 10000; k++) {
        sum += data[k];
      }
      return sum;
    });
  }
  for (MyFutureTask<Integer> MyFutureTask : tasks) {
    new Thread(MyFutureTask).start();
  }
  int threadSum = 0;
  for (MyFutureTask<Integer> MyFutureTask : tasks) {
    threadSum += MyFutureTask.get();
  }
  int sum = Arrays.stream(data).sum();
  System.out.println(sum == threadSum); // 输出结果为 true
}
```

## 总结

在本篇文章当中主要给大家介绍了`FutureTask`的内部原理，并且我们自己通过使用`ReentrantLock`和`Condition`实现了我们自己的`FutureTask`，本篇文章的主要内容如下：

- `FutureTask`的内部原理：
  - `FutureTask`首先会继承`Runnable`接口，这样就可以将`FutureTask`的对象直接放入`Thread`类当中，作为构造函数的参数。
  - 我们在使用`FutureTask`的时候需要传入一个`Callable`实现类的对象，在函数`call`当中实现我们需要执行的函数，执行完成之后，将`call`函数的返回值保存下来，当有线程调用`get`方法时候将保存的返回值返回。
- 我们使用条件变量进行对线程的阻塞和唤醒。
  - 当有线程调用`get`方法时，如果`call`已经执行完成，那么可以直接将结果返回，否则需要使用条件变量将线程挂起。
  - 当`call`函数执行完成的时候，需要使用条件变量将所有阻塞在`get`方法的线程唤醒。
- 双重检查：
  - 我们在`get`方法当中首先判断`returnVal`是否为空，如果不为空直接将结果返回，这就可以不用去竞争锁资源了，可以提高程序执行的效率。
  - 但是我们在使用锁保护的临界区还需要进行判断，判断`returnVal`是否为空，因为如果一个线程在第一次判断 `returnVal` 为空，然后这个时候它可能因为获取锁而被挂起， 而在被挂起的这段时间，call 可能已经执行完成，如果这个时候不进行判断直接执行 await方法，那后面这个线程将无法被唤醒，因为在`call`函数执行完成之后调用了`condition.signalAll()`，如果线程在这之后执行`await`方法，那么将来再没有线程去将这些线程唤醒。

---

更多精彩内容合集可访问项目：<https://github.com/Chang-LeHung/CSCore>

关注公众号：**一无是处的研究僧**，了解更多计算机（Java、Python、计算机系统基础、算法与数据结构）知识。

![](https://img2022.cnblogs.com/blog/2519003/202207/2519003-20220703200459566-1837431658.jpg)


# 从零自己动手写Lock Support

## 前言

在JDK当中给我们提供的各种并发工具当中，比如`ReentrantLock`等等工具的内部实现，经常会使用到一个工具，这个工具就是`LockSupport`。`LockSupport`给我们提供了一个非常强大的功能，他可以将一个线程阻塞也可以将一个线程唤醒，因此经常在并发的场景下进行使用。

## LockSupport实现原理

在了解LockSupport实现原理之前我们先用一个案例来了解一下LockSupport的功能！

```java
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.LockSupport;

public class Demo {

  public static void main(String[] args) throws InterruptedException {
    Thread thread = new Thread(() -> {
      System.out.println("park 之前");
      LockSupport.park(); // park 函数可以将调用这个方法的线程挂起
      System.out.println("park 之后");
    });
    thread.start();
    TimeUnit.SECONDS.sleep(5);
    System.out.println("主线程休息了 5s");
    System.out.println("主线程 unpark thread");
    LockSupport.unpark(thread); // 主线程将线程 thread 唤醒 唤醒之后线程 thread 才可以继续执行
  }
}

```

上面的代码的输出如下：

```java
park 之前
主线程休息了 5s
主线程 unpark thread
park 之后
```

咋一看上面的LockSupport的park和unpark实现的功能和await和signal实现的功能好像是一样的，但是其实不然，我们来看下面的代码：

```java

import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.LockSupport;

public class Demo02 {
  public static void main(String[] args) throws InterruptedException {
    Thread thread = new Thread(() -> {
      try {
        TimeUnit.SECONDS.sleep(5);
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
      System.out.println("park 之前");
      LockSupport.park(); // 线程 thread 后进行 park 操作 
      System.out.println("park 之后");
    });
    thread.start();
    System.out.println("主线程 unpark thread");
    LockSupport.unpark(thread); // 先进行 unpark 操作

  }
}
```

上面代码输出结果如下：

```java
主线程 unpark thread
park 之前
park 之后
```

在上面的代码当中主线程会先进行`unpark`操作，然后线程thread才进行`park`操作，这种情况下程序也可以正常执行。但是如果是signal的调用在await调用之前的话，程序则不会执行完成，比如下面的代码：

```java
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

public class Demo03 {

  private static final ReentrantLock lock = new ReentrantLock();
  private static final Condition condition = lock.newCondition();

  public static void thread() throws InterruptedException {
    lock.lock();

    try {
      TimeUnit.SECONDS.sleep(5);
      condition.await();
      System.out.println("等待完成");
    }finally {
      lock.unlock();
    }
  }

  public static void mainThread() {
    lock.lock();
    try {
      System.out.println("发送信号");
      condition.signal();
    }finally {
      lock.unlock();
      System.out.println("主线程解锁完成");
    }
  }

  public static void main(String[] args) {
    Thread thread = new Thread(() -> {
      try {
        thread();
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    });
    thread.start();

    mainThread();
  }
}

```

上面的代码输出如下：

```
发送信号
主线程解锁完成
```

在上面的代码当中“等待完成“始终是不会被打印出来的，这是因为signal函数的调用在await之前，signal函数只会对在它之前执行的await函数有效果，对在其后面调用的await是不会产生影响的。

那是什么原因导致的这个效果呢？

**其实JVM在实现LockSupport的时候，内部会给每一个线程维护一个计数器变量`_counter`，这个变量是表示的含义是“许可证的数量”，只有当许可证的数量大于等于0的时候线程才可以执行，同时许可证最大的数量只能为1。当调用一次park的时候许可证的数量会减一。当调用一次unpark的时候计数器就会加一，但是计数器的值不能超过1**。

- 如果我们在调用park的时候，计数器的值等于1，计数器的值变为-1，则线程可以继续执行。
- 如果我们在调用park的时候，计数器的值等于0，计数器的值变为-1，则线程不可以继续执行，需要将线程挂起。
- 如果我们在调用unpark的时候，被unpark的线程的计数器的值等于0，则需要将计数器的值变为1。
- 如果我们在调用unpark的时候，被unpark的线程的计数器的值等于1，则不需要改变计数器的值，因为计数器的最大值就是1。
- 如果我们在调用unpark的时候，被unpark的线程的计数器的值等于-1，则需要改变计数器的值，将计数器的值改成0，同时需要唤醒线程。

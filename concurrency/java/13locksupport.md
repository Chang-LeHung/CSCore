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


# 深入学习Synchronized各种使用方法



在Java当中synchronized通常是用来标记一个方法或者代码块。在Java当中被synchronized标记的代码或者方法在同一个时刻只能够有一个线程执行被synchronized修饰的方法或者代码块。因此被synchronized修饰的方法或者代码块不会出现**数据竞争**的情况，也就是说被synchronized修饰的代码块是并发安全的。

## Synchronized关键字

synchronized关键字通常使用在下面四个地方：

- synchronized修饰实例方法。
- synchronized修饰静态方法。
- synchronized修饰实例方法的代码块。
- synchronized修饰静态方法的代码块。

在实际情况当中我们需要仔细分析我们的需求选择合适的使用synchronized方法，在保证程序正确的情况下提升程序执行的效率。

## Synchronized修饰实例方法

下面是一个用Synchronized修饰实例方法的代码示例：

```java
public class SyncDemo {

  private int count;

  public synchronized void add() {
    count++;
  }

  public static void main(String[] args) throws InterruptedException {
    SyncDemo syncDemo = new SyncDemo();
    Thread t1 = new Thread(() -> {
      for (int i = 0; i < 10000; i++) {
        syncDemo.add();
      }
    });

    Thread t2 = new Thread(() -> {
      for (int i = 0; i < 10000; i++) {
        syncDemo.add();
      }
    });
    t1.start();
    t2.start();
    t1.join(); // 阻塞住线程等待线程 t1 执行完成
    t2.join(); // 阻塞住线程等待线程 t2 执行完成
    System.out.println(syncDemo.count);// 输出结果为 20000
  }
}

```

在上面的代码当中的`add`方法只有一个简单的`count++`操作，因为这个方法是使用`synchronized`修饰的因此每一个时刻只能有一个线程执行`add`方法，因此上面打印的结果是20000。如果`add`方法没有使用`synchronized`修饰的话，那么线程t1和线程t2就可以同时执行`add`方法，这回导致最终`count`的结果小于20000。


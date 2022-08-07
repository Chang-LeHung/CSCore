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

在上面的代码当中的`add`方法只有一个简单的`count++`操作，因为这个方法是使用`synchronized`修饰的因此每一个时刻只能有一个线程执行`add`方法，因此上面打印的结果是20000。如果`add`方法没有使用`synchronized`修饰的话，那么线程t1和线程t2就可以同时执行`add`方法，这可能会导致最终`count`的结果小于20000，因为`count++`操作不具备原子性。

上面的分析还是比较明确的，但是我们还需要知道的是`synchronized`修饰的`add`方法一个时刻只能有一个线程执行的意思是对于一个`SyncDemo`类的对象来说一个时刻只能有一个线程进入。比如现在有两个`SyncDemo`的对象`s1`和`s2`，一个时刻只能有一个线程进行`s1`的`add`方法，一个时刻只能有一个线程进入`s2`的`add`方法，但是同一个时刻可以有两个不同的线程执行`s1`和`s2`的`add`方法，也就说`s1`的`add`方法和`s2`的`add`是没有关系的，一个线程进入`s1`的`add`方法并不会阻止另外的线程进入`s2`的`add`方法，也就是说`synchronized`在修饰一个非静态方法的时候“锁”住的只是一个实例对象，并不会“锁”住其它的对象。其实这也很容易理解，一个实例对象是一个独立的个体别的对象不会影响他，他也不会影响别的对象。

## Synchronized修饰静态方法

Synchronized修饰静态方法：

```java
public class SyncDemo {

  private static int count;

  public static synchronized void add() {
    count++; // 注意 count 也要用 static 修饰 否则编译通过不了
  }

  public static void main(String[] args) throws InterruptedException {
    Thread t1 = new Thread(() -> {
      for (int i = 0; i < 10000; i++) {
        SyncDemo.add();
      }
    });

    Thread t2 = new Thread(() -> {
      for (int i = 0; i < 10000; i++) {
        SyncDemo.add();
      }
    });
    t1.start();
    t2.start();
    t1.join();
    t2.join();
    System.out.println(SyncDemo.count); // 输出结果为 20000
  }
}
```

上面的代码最终输出的结果也是20000，但是与前一个程序不同的是。这里的`add`方法用`static`修饰的，在这种情况下真正的只能有一个线程进入到`add`代码块，因为用`static`修饰的话是所有对象公共的，因此和前面的那种情况不同，不存在两个不同的线程同一时刻执行`add`方法。

你仔细想想如果能够让两个不同的线程执行`add`代码块，那么`count++`的执行就不是原子的了。那为什么没有用`static`修饰的代码为什么可以呢？因为当没有用`static`修饰时，每一个对象的`count`都是不同的，内存地址不一样，因此在这种情况下`count++`这个操作仍然是原子的！

## Sychronized修饰多个方法

synchronized修饰多个方法示例：

```java
public class AddMinus {
  public static int ans;

  public static synchronized void add() {
    ans++;
  }

  public static synchronized void minus() {
    ans--;
  }

  public static void main(String[] args) throws InterruptedException {
    Thread t1 = new Thread(() -> {
      for (int i = 0; i < 10000; i++) {
        AddMinus.add();
      }
    });

    Thread t2 = new Thread(() -> {
      for (int i = 0; i < 10000; i++) {
        AddMinus.minus();
      }
    });

    t1.start();
    t2.start();
    t1.join();
    t2.join();
    System.out.println(AddMinus.ans); // 输出结果为 0
  }
}
```

在上面的代码当中我们用`synchronized`修饰了两个方法，`add`和`minus`。这意味着在同一个时刻这两个函数只能够有一个被一个线程执行，也正是因为`add`和`minus`函数在同一个时刻只能有一个函数被一个线程执行，这才会导致`ans`最终输出的结果等于0。

对于一个实例对象来说：

```java
public class AddMinus {
  public int ans;

  public synchronized void add() {
    ans++;
  }

  public synchronized void minus() {
    ans--;
  }

  public static void main(String[] args) throws InterruptedException {
    AddMinus addMinus = new AddMinus();
    Thread t1 = new Thread(() -> {
      for (int i = 0; i < 10000; i++) {
        addMinus.add();
      }
    });

    Thread t2 = new Thread(() -> {
      for (int i = 0; i < 10000; i++) {
        addMinus.minus();
      }
    });

    t1.start();
    t2.start();
    t1.join();
    t2.join();
    System.out.println(addMinus.ans);
  }
}
```

上面的代码没有使用`static`关键字，因此我们需要`new`出一个实例对象才能够调用`add`和`minus`方法，但是同样对于`AddMinus`的实例对象来说同一个时刻只能有一个线程在执行`add`或者`minus`方法，因此上面代码的输出同样是0。


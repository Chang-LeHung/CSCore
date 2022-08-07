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

## Synchronized修饰实例方法代码块

Synchronized修饰实例方法代码块

```java
public class CodeBlock {

  private int count;

  public void add() {
    System.out.println("进入了 add 方法");
    synchronized (this) {
      count++;
    }
  }

  public void minus() {
    System.out.println("进入了 minus 方法");
    synchronized (this) {
        count--;
    }
  }

  public static void main(String[] args) throws InterruptedException {
    CodeBlock codeBlock = new CodeBlock();
    Thread t1 = new Thread(() -> {
      for (int i = 0; i < 10000; i++) {
        codeBlock.add();
      }
    });

    Thread t2 = new Thread(() -> {
      for (int i = 0; i < 10000; i++) {
        codeBlock.minus();
      }
    });

    t1.start();
    t2.start();
    t1.join();
    t2.join();
    System.out.println(codeBlock.count); // 输出结果为 0
  }
}
```

有时候我们并不需要用`synchronized`去修饰代码块，因为这样并发度就比较低了，一个方法一个时刻只能有一个线程在执行。因此我们可以选择用`synchronized`去修饰代码块，只让某个代码块一个时刻只能有一个线程执行，除了这个代码块之外的代码还是可以并行的。

比如上面的代码当中`add`和`minus`方法没有使用`synchronized`进行修饰，因此一个时刻可以有多个线程执行这个两个方法。在上面的`synchronized`代码块当中我们使用了`this`对象作为锁对象，只有拿到这个锁对象的线程才能够进入代码块执行，而在同一个时刻只能有一个线程能够获得锁对象。也就是说`add`函数和`minus`函数用`synchronized`修饰的两个代码块同一个时刻只能有一个代码块的代码能够被一个线程执行，因此上面的结果同样是0。

这里说的锁对象是`this`也就`CodeBlock`类的一个实例对象，因为它锁住的是一个实例对象，因此当实例对象不一样的时候他们之间是没有关系的，也就是说不同实例用`synchronized`修饰的代码块是没有关系的，他们之间是可以并发的。

## Synchronized修饰静态代码块

```java
public class CodeBlock {

  private static int count;

  public static void add() {
    System.out.println("进入了 add 方法");
    synchronized (CodeBlock.class) {
      count++;
    }
  }

  public static void minus() {
    System.out.println("进入了 minus 方法");
    synchronized (CodeBlock.class) {
        count--;
    }
  }

  public static void main(String[] args) throws InterruptedException {
    Thread t1 = new Thread(() -> {
      for (int i = 0; i < 10000; i++) {
        CodeBlock.add();
      }
    });

    Thread t2 = new Thread(() -> {
      for (int i = 0; i < 10000; i++) {
        CodeBlock.minus();
      }
    });

    t1.start();
    t2.start();
    t1.join();
    t2.join();
    System.out.println(CodeBlock.count);
  }
}
```

上面的代码是使用`synchronized`修饰静态代码块，上面代码的锁对象是`CodeBlock.class`，这个时候他不再是锁住一个对象了，而是一个类了，这个时候的并发度就变小了，上一份代码当锁对象是`CodeBlock`的实例对象时并发度更大一些，因为当锁对象时实例对象的时候，只有实例对象内部是不能够并发的，实例之间是可以并发的。但是当锁对象时`CodeBlock.class`的时候，实例对象之间时不能够并发的，因为这个时候的锁对象是一个类。

## 应该用什么对象作为锁对象

在前面的代码当中我们分别使用了实例对象和类的class对象作为锁对象，事实上你可以使用任何对象作为锁对象，但是不推荐使用字符串和基本类型的包装类作为锁对象，这是因为字符串对象和基本类型的包装对象会有缓存的问题。字符串有字符串常量池，整数有小整数池。因此在使用这些对象的时候他们可能最终都指向同一个对象，因为指向的都是同一个对象，线程获得锁对象的难度就会增加，程序的并发度就会降低。

比如在下面的示例代码当中就是由于锁对象是同一个对象而导致并发度下降：

```java
import java.util.concurrent.TimeUnit;

public class Test {

  public void testFunction() throws InterruptedException {
    synchronized ("HELLO WORLD") {
      System.out.println(Thread.currentThread().getName() + "\tI am in synchronized code block");
      TimeUnit.SECONDS.sleep(5);
    }
  }

  public static void main(String[] args) {
    Test t1 = new Test();
    Test t2 = new Test();
    Thread thread1 = new Thread(() -> {
      try {
        t1.testFunction();
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    });

    Thread thread2 = new Thread(() -> {
      try {
        t2.testFunction();
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    });
    thread1.start();
    thread2.start();
  }
}

```

在上面的代码当中我们使用两个不同的线程执行两个不同的对象内部的`testFunction`函数，按道理来说这两个线程是可以同时执行的，因为执行的是两个不同的实力对象的同步代码块。但是上面代码的执行首先一个线程会进入同步代码块然后打印输出，等待5秒之后，这个线程退出同步代码块另外一个线程才会再进入同步代码块，这就说明了两个线程不是同时执行的，其中一个线程需要等待另外一个线程执行完成才执行。这正是因为两个`Test`对象当中使用的`"HELLO WORLD"`字符串在内存当中是同一个对象，是存储在字符串常量池中的对象，这才导致了锁对象的竞争。

下面的代码执行的结果也是一样的，一个线程需要等待另外一个线程执行完成才能够继续执行，这是因为在Java当中如果整数数据在`[-128, 127]`之间的话使用的是小整数池当中的对象，这样可以减少频繁的内存申请和回收，对内存更加友好。

```java
import java.util.concurrent.TimeUnit;

public class Test {

  public void testFunction() throws InterruptedException {
    synchronized (Integer.valueOf(1)) {
      System.out.println(Thread.currentThread().getName() + "\tI am in synchronized code block");
      TimeUnit.SECONDS.sleep(5);
    }
  }

  public static void main(String[] args) {
    Test t1 = new Test();
    Test t2 = new Test();
    Thread thread1 = new Thread(() -> {
      try {
        t1.testFunction();
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    });

    Thread thread2 = new Thread(() -> {
      try {
        t2.testFunction();
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    });
    thread1.start();
    thread2.start();
  }
}
```


# 再谈线程池——深入剖析线程池的前世今生

## 深入剖析线程

灵魂质问：线程和进程的区别是什么？

一个非常经典的答案就是：进程是资源分配的最小单位，线程是操作系统调度的最小单位。

这句话是当然是正确的，但是如果你对整个计算机系统没有很好的理解的话你可能比较难去理解这句话。我们现在使用最简单的一个`Java`代码来看看一个`Java`代码是怎么被执行的。

```java
public class HelloWorld {

  public static void main(String[] args) {
    System.out.println("Hello World");
  }
}

```

我们可以使用`javac HelloWorld.java`编译这个文件，得到一个`HelloWorld.class`文件，然后我们执行`java HelloWorld`，然后就可以看到字符串的输出了。

![56](../../images/concurrency/56.png)

## 由线程到线程池

灵魂拷问：写了那么多代码，你能够描述线程在做一件什么事儿？

```java
public class Demo01 {

  public static void main(String[] args) {
    var thread = new Thread(() -> {
      System.out.println("Hello world from a Java thread");
    });
    thread.start();
  }
}
```

我们上面的这个用线程输出字符串的代码来进行说明。我们知道上面的Java代码启动了一个线程，然后执行`lambda`函数，在以前没有`lambda`表达式的时候我们可以使用匿名内部类实现，向下面这样。

```java
public class Demo01 {

  public static void main(String[] args) {
    var thread = new Thread(new Runnable() {
      @Override
      public void run() {
        System.out.println("Hello world from a Java thread");
      }
    });
    thread.start();
  }
}
```

但是本质上Java编译器在编译的时候都认为传递给他的是一个对象，然后执行对象的`run`方法。刚刚我们使用的`Thread`的构造函数如下：

```java
    public Thread(Runnable target) {
        this(null, target, "Thread-" + nextThreadNum(), 0);
    }
```

`Thread`在拿到这个对象的时候，当我们执行`Thread`的`start`方法的时候，会执行到一个`native`方法`start0`：

![56](../../images/concurrency/57.png)

![56](../../images/concurrency/58.png)

当JVM执行到这个方法的时候会调用操作系统给上层提供的API创建一个线程，然后这个线程会去解释执行我们之前给`Thread`对象传入的对象的`run`方法字节码，当`run`方法字节码执行完成之后，这个线程就会退出。

看到这里我们仔细思考一下线程在做一件什么样的事情，JVM给我们创建一个线程好像执行完一个函数（`run`）的字节码之后就退出了，线程的生命周期就结束了。确实是这样的，JVM给我们提供的线程就是去完成一个函数，然后退出（记住这一点，这一点很重要，为你后面理解线程池的原理有很大的帮助）。事实上JVM在使用操作系统给他提供的线程的时候也是给这个线程传递一个函数地址，然后让这个线程执行完这个函数，JVM给操作系统传递的函数，这个函数的功能就是去解释执行字节码，当解释执行字节码完成之后，这个函数也退出了。

看到这里可以将线程的功能总结成一句话：执行一个函数，当这个函数执行完成之后，线程就会退出，然后被回收，当然这个函数可以调用其他的函数。

## 线程池实现原理

## 总结


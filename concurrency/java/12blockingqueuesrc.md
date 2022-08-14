# JDK数组阻塞队列源码解析

## 前言

在前面一便文章[从零开始自己动手写阻塞队列](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486539&idx=1&sn=0bed9e23cf165be130a4f79e36e2c3f8&chksm=cf0c9042f87b195415d0278c0ce34d7f1c354b22246fe2101415174f9a8e3b74bc8830caf18e&token=2045951226&lang=zh_CN#rd)当中我们仔细介绍了阻塞队列提供给我们的功能，以及他的实现原理，并且基于谈到的内容我们自己实现了一个低配版的**数组阻塞队列**。在这篇文章当中我们将仔细介绍JDK具体是如何实现**数组阻塞队列**的。

## 阻塞队列的功能

而在本篇文章所谈到的阻塞队列当中，是在并发的情况下使用的，上面所谈到的是队列是**并发不安全**的，但是阻塞队列在并发下情况是安全的。阻塞队列的主要的需求如下：

- 队列基础的功能需要有，往队列当中放数据，从队列当中取数据。
- 所有的队列操作都要是**并发安全**的。
- 当队列满了之后再往队列当中放数据的时候，线程需要被挂起，当队列当中的数据被取出，让队列当中有空间的时候线程需要被唤醒。
- 当队列空了之后再往队列当中取数据的时候，线程需要被挂起，当有线程往队列当中加入数据的时候被挂起的线程需要被唤醒。
- 在我们实现的队列当中我们使用数组去存储数据，因此在构造函数当中需要提供数组的初始大小，设置用多大的数组。

上面就是数组阻塞队列给我们提供的最核心的功能，其中将线程挂起和唤醒就是阻塞队列的核心，挂起和唤醒体现了“阻塞”这一核心思想。

## 数组阻塞队列设计

阅读这部分内容你需要熟悉可重入锁`ReentrantLock`和条件变量`Condition`的使用。

### 字段设计

在JDK当中数组阻塞队列的实现是`ArrayBlockingQueue`类，在他的内部是使用数组实现的，我们现在来看一下它的主要的字段，为了方便阅读将所有的解释说明都写在的注释当中：

```java
    /** The queued items */
    final Object[] items; // 这个就是具体存储数据的数组

    /** items index for next take, poll, peek or remove */
    int takeIndex; // 因为是队列 因此我们需要知道下一个出队的数据的下标 这个就是表示下一个将要出队的数据的下标

    /** items index for next put, offer, or add */
    int putIndex; // 我们同时也需要下一个入队的数据的下标

    /** Number of elements in the queue */
    int count; // 统计队列当中一共有多少个数据

    /*
     * Concurrency control uses the classic two-condition algorithm
     * found in any textbook.
     */

    /** Main lock guarding all access */
    final ReentrantLock lock; // 因为阻塞队列是一种可以并发使用的数据结构

    /** Condition for waiting takes */
    private final Condition notEmpty; // 这个条件变量主要用于唤醒被 take 函数阻塞的线程 也就是从队列当中取数据的线程

    /** Condition for waiting puts */
    private final Condition notFull; // 这个条件变量主要用于唤醒被 put 函数阻塞的线程 也就是从队列当中放数据的线程

```

### 构造函数

构造函数的主要功能是申请指定大小的内存空间，并且对类的成员变量进行赋值操作。

```java
public ArrayBlockingQueue(int capacity) {
  // capacity 表示用与存储数据的数组的长度
  this(capacity, false);
}
// fair 这个参数主要是用于说明 是否使用公平锁
// 如果为 true 表示使用公平锁 执行效率弟 但是各个线程进入临界区的顺序是先来后到的顺序 更加公平
// 如果为 false 表示使用非公平锁 执行效率更高
public ArrayBlockingQueue(int capacity, boolean fair) {
  if (capacity <= 0)
    throw new IllegalArgumentException();
  this.items = new Object[capacity];
  // 对变量进行赋值操作
  lock = new ReentrantLock(fair);
  notEmpty = lock.newCondition();
  notFull =  lock.newCondition();
}

```

### 核心put函数

```java
public void put(E e) throws InterruptedException {
  checkNotNull(e);
  final ReentrantLock lock = this.lock;
  lock.lockInterruptibly();
  try {
    while (count == items.length)
      notFull.await();
    enqueue(e);
  } finally {
    lock.unlock();
  }
}

private void enqueue(E x) {
  // assert lock.getHoldCount() == 1;
  // assert items[putIndex] == null;
  final Object[] items = this.items;
  items[putIndex] = x;
  if (++putIndex == items.length)
    putIndex = 0;
  count++;
  notEmpty.signal();
}
```


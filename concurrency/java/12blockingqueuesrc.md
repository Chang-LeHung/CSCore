# JDK数组阻塞队列源码深入剖析

## 前言

在前面一篇文章[从零开始自己动手写阻塞队列](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486539&idx=1&sn=0bed9e23cf165be130a4f79e36e2c3f8&chksm=cf0c9042f87b195415d0278c0ce34d7f1c354b22246fe2101415174f9a8e3b74bc8830caf18e&token=2045951226&lang=zh_CN#rd)当中我们仔细介绍了阻塞队列提供给我们的功能，以及他的实现原理，并且基于谈到的内容我们自己实现了一个低配版的**数组阻塞队列**。在这篇文章当中我们将仔细介绍JDK具体是如何实现**数组阻塞队列**的。

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

### 数组的循环使用

因为我们是使用数组存储队列当中的数据，从下表为0的位置开始，当我们往队列当中加入一些数据之后，队列的情况可能如下，其中head表示队头，tail表示队尾。

<img src="../../images/arraydeque/26.png" alt="24" style="zoom:80%;" />

在上图的基础之上我们在进行四次出队操作，结果如下：

<img src="../../images/arraydeque/27.png" alt="24" style="zoom:80%;" />

在上面的状态下，我们继续加入8个数据，那么布局情况如下：

<img src="../../images/arraydeque/28.png" alt="24" style="zoom:80%;" />

我们知道上图在加入数据的时候不仅将数组后半部分的空间使用完了，而且可以继续使用前半部分没有使用过的空间，也就是说在队列内部实现了一个循环使用的过程。

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
// 如果为 true 表示使用公平锁 执行效率低 但是各个线程进入临界区的顺序是先来后到的顺序 更加公平
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

### put函数

这个函数是阻塞队列对核心的函数之一了，首先我们需要了解的是，如果一个线程调用了这个函数往队列当中加入数据，如果此时队列已经满了则线程需要被挂起，如果没有满则需要将数据加入到队列当中，也就是将数据存储到数组当中。注意还有一个很重要的一点是，当我们往队列当中加入一个数据之后需要发一个信号给其他被`take`函数阻塞的线程，因为这些线程在取数据的时候可能队列当中已经空了，因此需要将这些线程唤醒。

```java
public void put(E e) throws InterruptedException {
  checkNotNull(e); // 保证输入的数据不为 null 代码在下方
  final ReentrantLock lock = this.lock;
  // 进行加锁操作，因为下面是临界区
  lock.lockInterruptibly();
  try {
    while (count == items.length) // 如果队列已经满了 也就是队列当中数据的个数 count == 数组的长度的话 就需要将线程挂起
      notFull.await();
    // 当队列当中有空间的之后将数据加入到队列当中 这个函数在下面仔细分析 代码在下方
    enqueue(e);
  } finally {
    lock.unlock();
  }
}

private static void checkNotNull(Object v) {
  if (v == null)
    throw new NullPointerException();
}

private void enqueue(E x) {
  // assert lock.getHoldCount() == 1;
  // assert items[putIndex] == null;
  // 进入这个函数的线程已经在 put 函数当中加上锁了 因此这里不需要加锁
  final Object[] items = this.items;
  items[putIndex] = x;
  if (++putIndex == items.length) // 因为这个数据是循环使用的 因此可以回到下标为0的位置
    // 因为队列当中的数据可以出队 因此下标为 0 的位置不存在数据可以使用
    putIndex = 0;
  count++;
  // 在这里需要将一个被 take 函数阻塞的线程唤醒 如果调用这个方法的时候没有线程阻塞
  // 那么调用这个方法相当于没有调用 如果有线程阻塞那么将会唤醒一个线程
  notEmpty.signal();
}
```

**注意**：这里有一个地方非常容易被忽略，那就是在将线程挂起的时候使用的是`while`循环而不是`if`条件语句，代码：

```java
final ReentrantLock lock = this.lock;
lock.lockInterruptibly();
try {
  while (count == items.length)
    notFull.await();
  enqueue(e);
} finally {
  lock.unlock();
}
```

这是因为，线程被唤醒之后并不会立马执行，因为线程在调用`await`方法的之后会释放锁🔒，他想再次执行还需要再次获得锁，然后就在他获取锁之前的这段时间里面，可能其他的线程也会从数组当中放数据，因此这个线程执行的时候队列可能还是满的，因此需要再次判断，否则就会覆盖数据，像这种唤醒之后并没有满足线程执行条件的现象叫做**虚假唤醒**，因此大家在写程序的时候要格外注意，当需要将线程挂起或者唤醒的之后，最好考虑清楚，如果不确定可以使用`while`替代`if`，这样的话更加保险。

### take函数

这个函数主要是从队列当中取数据，但是当队列为空的时候需要将调用这个方法的线程阻塞。当队列当中有数据的时候，就可以从队列当中取出数据，但是有一点很重要的就是当从队列当中取出数据之后，需要调用`signal`方法，用于唤醒被 put 函数阻塞的线程，因为从队列当中取出数据了，队列肯定已经不满了，因此可以唤醒被 put 函数阻塞的线程了。

```java
public E take() throws InterruptedException {
  final ReentrantLock lock = this.lock;
  // 因为取数据的代码涉及到数据竞争 也就是说多个线程同时竞争 数组数据items 因此需要用锁保护起来
  lock.lockInterruptibly();
  try {
    // 当 count == 0 说明队列当中没有数据
    while (count == 0)
      notEmpty.await();
    // 当队列当中还有数据的时候可以将数据出队
    return dequeue();
  } finally {
    lock.unlock();
  }
}

private E dequeue() {
  // assert lock.getHoldCount() == 1;
  // assert items[takeIndex] != null;
  final Object[] items = this.items;
  @SuppressWarnings("unchecked")
  // 取出数据
  E x = (E) items[takeIndex];
  items[takeIndex] = null; // 将对应的位置设置为 null 数据就可以被垃圾收集器回收了
  if (++takeIndex == items.length)
    takeIndex = 0;
  count--;
  // 迭代器也需要出队 如果不了
  if (itrs != null)
    itrs.elementDequeued();
  // 调用 signal 函数 将被 put 函数阻塞的线程唤醒 如果调用这个方法的时候没有线程阻塞
  // 那么调用这个方法相当于没有调用 如果有线程阻塞那么将会唤醒一个线程
  notFull.signal();
  return x;
}

```

同样的道理这里也需要使用`while`循环去进行阻塞，否则可能存在**虚假唤醒**，可能队列当中没有数据返回的数据为 null，而且会破坏队列的结构因为会涉及队列的两个端点的值的改变，也就是`takeIndex`和`putIndex`的改变。

### offer函数

这个函数的作用和put函数一样，只不过当队列满了的时候，这个函数返回false，加入数据成功之后这个函数返回true，下面的代码就比较简单了。

```java
public boolean offer(E e) {
  checkNotNull(e);
  final ReentrantLock lock = this.lock;
  lock.lock();
  try {
    // 如果队列当中的数据个数和数组的长度相等 说明队列满了 直接返回 false 即可
    if (count == items.length)
      return false;
    else {
      enqueue(e);
      return true;
    }
  } finally {
    lock.unlock();
  }
}

```

### add函数

这个函数和上面两个函数的意义也是一样的，只不过当队列满了之后这个函数会抛出异常。

```java
public boolean add(E e) {
  if (offer(e))
    return true;
  else
    throw new IllegalStateException("Queue full");
}

```

### poll函数

这个函数和take函数的作用差不多，但是这个函数不会阻塞，当队列当中没有数据的时候直接返回null，有数据的话返回数据。

```java
public E poll() {
  final ReentrantLock lock = this.lock;
  lock.lock();
  try {
    return (count == 0) ? null : dequeue();
  } finally {
    lock.unlock();
  }
}

```

## 总结

在本篇文章当中主要介绍了JDK内部是如何实现`ArrayBlockingQueue`的，如果你对锁和队列的使用有一定的了解本篇文章应该还是比较容易理解的。在实现`ArrayBlockingQueue`当中有以下需要注意的点：

- `put`函数，如果在往队列当中加入数据的时候队列满了，则需要将线程挂起。在队列当中有空间之后，线程被唤醒继续执行，在往队列当中加入了数据之后，需要调用`signal`方法，唤醒被`take`函数阻塞的线程。
- `take`函数，如果在往队列当中取出数据的时候队列空了，则需要将线程挂起。在队列当中有数据之后，线程被唤醒继续执行，在从队列当中取出数据之后，需要调用`signal`方法，唤醒被`put`函数阻塞的线程。
- 在调用`await`函数的时候，需要小心**虚假唤醒**现象。

---

以上就是本篇文章的所有内容了，我是**LeHung**，我们下期再见！！！更多精彩内容合集可访问项目：<https://github.com/Chang-LeHung/CSCore>

关注公众号：**一无是处的研究僧**，了解更多计算机（Java、Python、计算机系统基础、算法与数据结构）知识。

![](https://img2022.cnblogs.com/blog/2519003/202207/2519003-20220703200459566-1837431658.jpg)


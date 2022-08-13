# 从零开始自己动手写阻塞队列

## 前言

在我们平时编程的时候一个很重要的工具就是容器，在本篇文章当中主要给大家介绍阻塞队列的原理，并且在了解原理之后自己动手实现一个低配版的阻塞队列。

## 需求分析

在前面的两片文章[ArrayDeque（JDK双端队列）源码深度剖析](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484612&idx=1&sn=63a5a21fab640619333d9836a000ea44&chksm=cf0c98cdf87b11db7d63b2d028f0a70ea73e7c84338bce8e7cdf4bee9d5b0c7ec2bf23663233&token=1311889589&lang=zh_CN#rd)和[深入剖析（JDK）ArrayQueue源码](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484813&idx=1&sn=ace534e0492bbc9f77b9cf488c7c4edc&chksm=cf0c9984f87b109210387f71d9591450ead667611351cd6857b7accb6f1fdc02939e12ad0434&token=969171239&lang=zh_CN#rd)当中我们仔细介绍了队列的原理，如果大家感兴趣可以查看一下！

而在本篇文章所谈到的阻塞队列当中，是在并发的情况下使用的，上面所谈到的是队列是**并发不安全**的，但是阻塞队列在并发下情况是安全的。阻塞队列的主要的需求如下：

- 队列基础的功能需要有，往队列当中放数据，从队列当中取数据。
- 所有的队列操作都要是**并发安全**的。
- 当队列满了之后再往队列当中放数据的时候，线程需要被挂起，当队列当中的数据被取出，让队列当中有空间的时候线程需要被唤醒。
- 当队列空了之后再往队列当中取数据的时候，线程需要被挂起，当有线程往队列当中加入数据的时候被挂起的线程需要被唤醒。
- 在我们实现的队列当中我们使用数组去存储数据，因此在构造函数当中需要提供数组的初始大小，设置用多大的数组。

## 阻塞队列实现原理

### 线程阻塞和唤醒

在上面我们已经谈到了阻塞队列是**并发安全**的，而且我们还有将线程唤醒和阻塞的需求，因此我们可以选择可重入锁`ReentrantLock`保证并发安全，但是我们还需要将线程唤醒和阻塞，因此我们可以选择条件变量`Condition`进行线程的唤醒和阻塞操作，在`Condition`当中我们将会使用到的，主要有以下两个函数：

- `signal`用于唤醒线程，当一个线程调用`Condition`的`signal`函数的时候就可以唤醒一个被`await`函数阻塞的线程。
- `await`用于阻塞线程，当一个线程调用`Condition`的`await`函数的时候这个线程就会阻塞。

### 数组循环使用

因为队列是一端进一端出，因此队列肯定有头有尾。

<img src="../../images/arraydeque/24.png" alt="24" style="zoom:80%;" />

当我们往队列当中加入一些数据之后，队列的情况可能如下：

<img src="../../images/arraydeque/26.png" alt="24" style="zoom:80%;" />

在上图的基础之上我们在进行四次出队操作，结果如下：

<img src="../../images/arraydeque/27.png" alt="24" style="zoom:80%;" />

在上面的状态下，我们继续加入8个数据，那么布局情况如下：

<img src="../../images/arraydeque/28.png" alt="24" style="zoom:80%;" />

我们知道上图在加入数据的时候不仅将数组后半部分的空间使用完了，而且可以继续使用前半部分没有使用过的空间，也就是说在队列内部实现了一个循环使用的过程。

为了保证数组的循环使用，我们需要用一个变量记录队列头在数组当中的位置，用一个变量记录队列尾部在数组当中的位置，还需要有一个变量记录队列当中有多少个数据。

## 代码实现

### 成员变量定义

根据上面的分析我们可以知道，在我们自己实现的类当中我们需要有如下的类成员变量：

```java
// 用于保护临界区的锁
private final ReentrantLock lock;
// 用于唤醒取数据的时候被阻塞的线程
private final Condition notEmpty;
// 用于唤醒放数据的时候被阻塞的线程
private final Condition notFull;
// 用于记录从数组当中取数据的位置 也就是队列头部的位置
private int takeIndex;
// 用于记录从数组当中放数据的位置 也就是队列尾部的位置
private int putIndex;
// 记录队列当中有多少个数据
private int count;
// 用于存放具体数据的数组
private Object[] items;
```

### 构造函数

我们的构造函数也很简单，最核心的就是传入一个数组大小的参数，并且给上面的变量进行初始化赋值。

```java
@SuppressWarnings("unchecked")
public MyArrayBlockingQueue(int size) {
  this.lock = new ReentrantLock();
  this.notEmpty = lock.newCondition();
  this.notFull = lock.newCondition();
  // 其实可以不用初始化 类会有默认初始化 默认初始化为0
  takeIndex = 0;
  putIndex = 0;
  count = 0;
  // 数组的长度肯定不能够小于0
  if (size <= 0)
    throw new RuntimeException("size can not be less than 1");
  items = (E[])new Object[size];
}

```

### put函数

这是一个比较重要的函数了，在这个函数当中如果队列没有满，则直接将数据放入到数组当中即可，如果数组满了，则需要将线程挂起。

```java
public void put(E x){
  // put 函数可能多个线程调用 但是我们需要保证在给变量赋值的时候只能够有一个线程
  // 因为如果多个线程同时进行赋值的话 那么可能后一个线程的赋值操作覆盖了前一个线程的赋值操作
  // 因此这里需要上锁
  lock.lock();

  try {
    // 如果队列当中的数据个数等于数组的长度的话 说明数组已经满了
    // 这个时候需要将线程挂起
    while (count == items.length)
      notFull.await();
    // 当数组没有满 或者在挂起之后再次唤醒的话说明数组当中有空间了
    // 这个时候需要将数组入队 
    // 调用入队函数将数据入队
    enqueue(x);
  } catch (InterruptedException e) {
    e.printStackTrace();
  } finally {
    // 解锁
    lock.unlock();
  }
}

// 将数据入队
private void enqueue(E x) {
  this.items[putIndex] = x;
  if (++putIndex == items.length)
    putIndex = 0;
  count++;
  notEmpty.signal();
}

```

### offer函数

offer函数和put函数一样，但是与put函数不同的是，当数组当中数据填满之后offer函数返回`false`，而不是被阻塞。

```java
public boolean offer(E e) {
  final ReentrantLock lock = this.lock;
  lock.lock();
  try {
    // 如果数组满了 则直接返回false 而不是被阻塞
    if (count == items.length)
      return false;
    else {
      // 如果数组没有满则直接入队 并且返回 true
      enqueue(e);
      return true;
    }
  } finally {
    lock.unlock();
  }
}

```

### add函数

这个函数和上面两个函数作用一样，也是往队列当中加入数据，但是单队列满了之后这个函数会抛出异常。

```java
public boolean add(E e) {
  if (offer(e))
    return true;
  else
    throw new RuntimeException("Queue full");
}

```

### take函数

```java
public E take() throws InterruptedException {
  // 这个函数也是不能够并发的
  // 进行加锁操作
  lock.lock();
  try {
    // 当 count 等于0 说明队列为空
    // 需要将线程挂起等待
    while (count == 0)
      notEmpty.await();
    // 当被唤醒之后进行出队操作
    return dequeue();
  }finally {
    lock.unlock();
  }
}

private E  dequeue() {
  final Object[] items = this.items;
  @SuppressWarnings("unchecked")
  E x = (E) items[takeIndex];
  items[takeIndex] = null; // 将对应的位置设置为 null GC就可以回收了
  if (++takeIndex == items.length)
    takeIndex = 0;
  count--; // 队列当中数据少一个了
  // 因为出队了一个数据 可以唤醒一个被 put 函数阻塞的线程 如果这个时候没有被阻塞的线程
  // 这个函数就不会起作用 也就说在这个函数调用之后被 put 函数挂起的线程也不会被唤醒
  notFull.signal();
  return x;
}
```


# ArrayDeque（JDK双端队列）源码深度剖析

## 前言

在本篇文章当中主要跟大家介绍`JDK`给我们提供的一种用数组实现的**双端队列**，在之前的文章[LinkedList源码剖析当中](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247483907&idx=1&sn=6281a11e6ed1917ecb3a10319474193d&chksm=cf0c9e0af87b171c7193949b5b7eb0b8f813b05b3d3b96ea784df86a3dde4286ad03908122da&token=883596793&lang=zh_CN#rd)我们已经介绍了一种**双端队列**，不过与`ArrayDeque`不同的是，`LinkedList`的双端队列使用双向链表实现的。

## 双端队列整体分析

我们通常所谈论到的队列都是一端进一端出，而双端队列的两端则都是可进可出。下面是双端队列的几个操作：

- 数据从双端队列左侧进入。

<img src="../images/arraydeque/02.png" alt="01" style="zoom:80%;" />

- 数据从双端队列右侧进入。

<img src="../images/arraydeque/03.png" alt="01" style="zoom:80%;" />

- 数据从双端队列左侧弹出。

<img src="../images/arraydeque/04.png" alt="01" style="zoom:80%;" />

- 数据从双端队列右侧弹出。

<img src="../images/arraydeque/05.png" alt="01" style="zoom:80%;" />

而在`ArrayDeque`当中也给我们提供了对应的方法去实现，比如下面这个例子就是上图对应的代码操作：

```java
public void test() {
    ArrayDeque<Integer> deque = new ArrayDeque<>();
    deque.addLast(100);
    System.out.println(deque);
    deque.addFirst(55);
    System.out.println(deque);
    deque.addLast(-55);
    System.out.println(deque);
    deque.removeFirst();
    System.out.println(deque);
    deque.removeLast();
    System.out.println(deque);
}
// 输出结果
[100]
[55, 100]
[55, 100, -55]
[100, -55]
[100]
```



## 数组实现ArrayDeque(双端队列)的原理

`ArrayDeque`底层是使用数组实现的，而且数组的长度必须是`2`的整数次幂，这么操作的原因是为了后面位运算好操作。在`ArrayDeque`当中有两个整形变量`head`和`tail`，分别指向右侧的第一个进入队列的数据和左侧第一个进行队列的数据，整个内存布局如下图所示：

<img src="../images/arraydeque/06.png" alt="01" style="zoom:80%;" />

其中`tail`指的位置没有数据，`head`指的位置存在数据。

- 当我们需要从左往右增加数据时（入队），内存当中数据变化情况如下：

<img src="../images/arraydeque/14.png" alt="01" style="zoom:80%;" />

- 当我们需要从右往做左增加数据时（入队），内存当中数据变化情况如下：

<img src="../images/arraydeque/15.png" alt="01" style="zoom:80%;" />

- 当我们需要从右往左删除数据时（出队），内存当中数据变化情况如下：

<img src="../images/arraydeque/13.png" alt="01" style="zoom:80%;" />

- 当我们需要从左往右删除数据时（出队），内存当中数据变化情况如下：

<img src="../images/arraydeque/12.png" alt="01" style="zoom:80%;" />

## ArrayDeque类关键字段分析

```java
// 底层用于存储具体数据的数组
transient Object[] elements;
// 这就是前面谈到的 head
transient int head;
// 与上文谈到的 tail 含义一样
transient int tail;
// MIN_INITIAL_CAPACITY 表示数组 elements 的最短长度
private static final int MIN_INITIAL_CAPACITY = 8;
```

以上就是`ArrayDeque`当中的最主要的字段，其含义还是比较容易理解的！

## ArrayDeque构造函数分析

- 默认构造函数，数组默认申请的长度为`16`。

```java
public ArrayDeque() {
    elements = new Object[16];
}
```

- 指定数组长度的初始化长度，下面列出了改构造函数涉及的所有函数。

```java
public ArrayDeque(int numElements) {
    allocateElements(numElements);
}

private void allocateElements(int numElements) {
    elements = new Object[calculateSize(numElements)];
}
private static int calculateSize(int numElements) {
    int initialCapacity = MIN_INITIAL_CAPACITY;
    // Find the best power of two to hold elements.
    // Tests "<=" because arrays aren't kept full.
    if (numElements >= initialCapacity) {
        initialCapacity = numElements;
        initialCapacity |= (initialCapacity >>>  1);
        initialCapacity |= (initialCapacity >>>  2);
        initialCapacity |= (initialCapacity >>>  4);
        initialCapacity |= (initialCapacity >>>  8);
        initialCapacity |= (initialCapacity >>> 16);
        initialCapacity++;

        if (initialCapacity < 0)   // Too many elements, must back off
            initialCapacity >>>= 1;// Good luck allocating 2 ^ 30 elements
    }
    return initialCapacity;
}
```

上面的最难理解的就是函数`calculateSize`了，他的主要作用是如果用户输入的长度小于`MIN_INITIAL_CAPACITY`时，返回`MIN_INITIAL_CAPACITY`。否则返回比`initialCapacity`大的第一个是`2`的整数幂的整数，比如说如果输入的是`9`返回的`16`，输入`4`返回`8`。

`calculateSize`的代码还是很难理解的，让我们一点一点的来分析。首先我们使用一个`2`的整数次幂的数进行上面`移位操作`的操作！

<img src="../images/hashmap/01-hashmap17.png" style="zoom:80%;" />

<img src="../images/hashmap/01-hashmap21.png" style="zoom:80%;" />

从上图当中我们会发现，我们在一个数的二进制数的32位放一个`1`，经过移位之后最终`32`位的比特数字全部变成了`1`。根据上面数字变化的规律我们可以发现，任何一个比特经过上面移位的变化，这个比特后面的`31`个比特位都会变成`1`，像下图那样：

<img src="../images/hashmap/01-hashmap23.png" style="zoom:80%;" />

因此上述的移位操作的结果只取决于最高一位的比特值为`1`，移位操作后它后面的所有比特位的值全为`1`，而在上面函数的最后，我们返回的结果就是上面移位之后的结果 `+1`。又因为移位之后最高位的`1`到最低位的`1`之间的比特值全为`1`，当我们`+1`之后他会不断的进位，最终只有一个比特位置是`1`，因此它是`2`的整数倍。

<img src="../images/hashmap/01-hashmap24.png" style="zoom:80%;" />

经过上述过程分析，我们就可以立即函数`calculateSize`了。

## ArrayDeque关键函数分析

### addLast函数分析

```java
// tail 的初始值为 0 
public void addLast(E e) {
    if (e == null)
        throw new NullPointerException();
    elements[tail] = e;
    // 这里进行的 & 位运算 相当于取余数操作
    // (tail + 1) & (elements.length - 1) == (tail + 1) % elements.length
    if ( (tail = (tail + 1) & (elements.length - 1)) == head)
        doubleCapacity();
}
```

代码`(tail + 1) & (elements.length - 1) == (tail + 1) % elements.length`成立的原因是任意一个数$a$对$2^n$进行取余数操作和$a$跟$2^n - 1$进行`&`运算的结果相等，即：
$$
a\% 2^n = a \& (2^n - 1)
$$

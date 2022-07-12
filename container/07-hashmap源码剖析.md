# `HashMap`源码深度剖析，手把手带你分析每一行代码！

在前面的两篇文章[哈希表的原理](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484235&idx=1&sn=e4fda2cd7520d2d68d7a3c179c8845b3&chksm=cf0c9f42f87b1654e49e21d043fed104ce5fd4839f2eae13cd95c7630fd547e208a1318fd8d3&token=1155116583&lang=zh_CN#rd)和[200行代码带你写自己的`HashMap`](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484235&idx=1&sn=e4fda2cd7520d2d68d7a3c179c8845b3&chksm=cf0c9f42f87b1654e49e21d043fed104ce5fd4839f2eae13cd95c7630fd547e208a1318fd8d3&token=1155116583&lang=zh_CN#rd)当中我们仔细谈到了哈希表的原理并且自己动手使用线性探测法实现了我们自己的哈希表`MyHashMap`。在本篇文章当中我们将仔细分析`JDK`当中`HashMap`的源代码。

首先我们需要了解的是一个容器最重要的四个功能 `增删改查` ，而我们也是主要根据这四个功能进行展开一步一步的剖析`HashMap`的源代码。在正式进行源码分析之前，先提一下：在`JDK`当中实现的`HashMap`解决哈希冲突的办法是使用`链地址法`，而我们自己之前在文章[200行代码带你写自己的`HashMap`](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484235&idx=1&sn=e4fda2cd7520d2d68d7a3c179c8845b3&chksm=cf0c9f42f87b1654e49e21d043fed104ce5fd4839f2eae13cd95c7630fd547e208a1318fd8d3&token=1155116583&lang=zh_CN#rd)当中实现的`MyHashMap`解决哈希冲突的办法是线性探测法，大家注意一下这两种方法的不同。

## `HashMap`源码类中关键字段分析

- 下面字段表示默认的哈希表的长度，也就是`HashMap`底层使用数组的默认长度，在`HashMap`当中底层所使用的的数组的长度必须是`2`的整数次幂，这一点我们在文章[200行代码带你写自己的`HashMap`](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484235&idx=1&sn=e4fda2cd7520d2d68d7a3c179c8845b3&chksm=cf0c9f42f87b1654e49e21d043fed104ce5fd4839f2eae13cd95c7630fd547e208a1318fd8d3&token=1155116583&lang=zh_CN#rd)已经仔细做出了说明。

```java
    /**
     * The default initial capacity - MUST be a power of two.
     */
    static final int DEFAULT_INITIAL_CAPACITY = 1 << 4; // aka 16
```

- 这个字段表示哈希表当中数组的最大长度，`HashMap`底层使用的数组长度不能超过这个值。

```java
    /**
     * The maximum capacity, used if a higher value is implicitly specified
     * by either of the constructors with arguments.
     * MUST be a power of two <= 1<<30.
     */
    static final int MAXIMUM_CAPACITY = 1 << 30;
```

- 字段`DEFAULT_LOAD_FACTOR`的作用表示在`HashMap`当中默认的负载因子的值。

```java
    /**
     * The load factor used when none specified in constructor.
     */
    static final float DEFAULT_LOAD_FACTOR = 0.75f;
```

在实际情况当中我们并不是当`HashMap`当中的数组完全被使用完之后才进行扩容，因为如果数组快被使用完之后，再加入数据产生哈希冲突的可能性就会很大，因此我们通常会设置一个负载因子`(load factor)`，当数组的使用率超过这个值的时候就进行扩容，即当(数组长度为`L`，数组当中数据个数为`S`，负载因子为`F`)：
$$
S \ge L \times F
$$

- `TREEIFY_THRESHOLD` 这个字段主要表示将链表（在`JDK`当中是采用链地址法去解决哈希冲突的问题）变成一个红黑树（如果你不了解红黑树，可以将其认为是一种平衡二叉树）的条件，在`JDK1.8`之后`JDK`中实现`HashMap`不仅采用链地址法去解决哈希冲突，而且链表满足一定条件之后会将链表变成一颗红黑树。而将链表变成一颗红黑树的`必要条件`是链表当中数据的个数要大于等于`TREEIFY_THRESHOLD`，请大家注意是`必要条件`不是`充分条件`，也就是说满足这个条件还不行，它还需要满足另外一个条件，就是哈希表中数组的长度要大于等于`MIN_TREEIFY_CAPACITY`，`MIN_TREEIFY_CAPACITY`在`JDK`当中的默认值是64。

```java
    /**
     * The bin count threshold for using a tree rather than list for a
     * bin.  Bins are converted to trees when adding an element to a
     * bin with at least this many nodes. The value must be greater
     * than 2 and should be at least 8 to mesh with assumptions in
     * tree removal about conversion back to plain bins upon
     * shrinkage.
     */
    static final int TREEIFY_THRESHOLD = 8;

    /**
     * The smallest table capacity for which bins may be treeified.
     * (Otherwise the table is resized if too many nodes in a bin.)
     * Should be at least 4 * TREEIFY_THRESHOLD to avoid conflicts
     * between resizing and treeification thresholds.
     */
    static final int MIN_TREEIFY_CAPACITY = 64;

```

- 
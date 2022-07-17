## 深入剖析（JDK）ArrayQueue源码

## 前言

在本篇文章当中主要给大家介绍一个比较简单的`JDK`为我们提供的容器`ArrayQueue`，这个容器主要是用数组实现的一个单向队列，整体的结构相对其他容器来说就比较简单了。

## ArrayQueue内部实现

首先`ArrayQueue`内部是由循环数组实现的，可能保证增加和删除数据的时间复杂度都是$O(1)$，不像`ArrayList`删除数据的时间复杂度为$O(n)$。在`ArrayQueue`内部有两个整型数据`head`和`tail`，这两个的作用主要是指向队列的头部和尾部，它的初始状态在内存当中的布局如下图所示：

<img src="../images/arraydeque/24.png" alt="24" style="zoom:80%;" />

因为是初始状态`head`和`tail`的值都等于0，指向数组当中第一个数据。现在我们向`ArrayQueue`内部加入5个数据，那么他的内存布局将如下图所示：

<img src="../images/arraydeque/26.png" alt="24" style="zoom:80%;" />



## ArrayQueue源码剖析

## 总结




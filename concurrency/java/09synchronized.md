# Synchronized锁升级原理深入剖析

## 前言

在上篇文章[深入学习Synchronized各种使用方法](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486361&idx=1&sn=dce819edcce0d509d7fec212abc2bd03&chksm=cf0c9790f87b1e86fcef49c58cbad141a43e803f01fec8a2170c82fae25a284601c5f9f27929&token=1166204888&lang=zh_CN#rd)当中我们仔细介绍了在各种情况下该如何使用synchronized关键字。因为在我们写的程序当中可能会经常使用到synchronized关键字，因此JVM对synchronized做出了很多优化，而在本篇文章当中我们将仔细介绍JVM对synchronized的各种优化的细节。

## 基础准备

在正式谈synchronized的原理之前我们先谈一下自旋锁，因为在synchronized的优化当中
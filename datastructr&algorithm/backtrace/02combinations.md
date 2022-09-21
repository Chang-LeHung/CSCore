# 组合总和

## 前言

在上篇文章[通过组合问题看透回溯法](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486751&idx=1&sn=8e9cedd729d01ff8867fcb2c085ecbe3&chksm=cf0c9116f87b18002551eefcd773f4762d79d2c06614b304437bfcd382e14afa031d29cc4dcc&mpshare=1&scene=22&srcid=0921yRhazbXlqGuEZagKb0p9&sharer_sharetime=1663737871952&sharer_shareid=236a49567847c05f78e6b440ce6dabff#rd)当中我们通过介绍一个组合问题，仔细地分析了组合问题的回溯过程。我们之后会继续介绍一些比较经典的回溯算法题，帮助深入彻底理解回溯算法的执行过程和原理，如果对回溯的整个过程还不是很了解的话可以先阅读上面那篇文章。

## [组合总和](https://leetcode.cn/problems/combination-sum-ii/)

>给你一个 无重复元素 的整数数组 candidates 和一个目标整数 target ，找出 candidates 中可以使数字和为目标数 target 的 所有 不同组合 ，并以列表形式返回。你可以按 任意顺序 返回这些组合。
>
>candidates 中的 同一个 数字可以 无限制重复被选取 。如果至少一个数字的被选数量不同，则两种组合是不同的。 
>
>对于给定的输入，保证和为 target 的不同组合数少于 150 个。

例1：

```JAVA
输入：candidates = [2,3,6,7], target = 7
输出：[[2,2,3],[7]]
解释：
2 和 3 可以形成一组候选，2 + 2 + 3 = 7 。注意 2 可以使用多次。
7 也是一个候选， 7 = 7 。
仅有这两种组合。
```

例2：

```JAVA
输入: candidates = [2,3,5], target = 8
输出: [[2,2,2,2],[2,3,3],[3,5]]
```

## 解法

根据题目的意思我们是需要从给定的集合当中选出几个数据，而且可以进行重复选取，只需要保证被我们选中的数据的和为`target`即可。对于这个问题我们可以构造一个如下图所示的解树：

![11](../../images/backtrace/13.png)



![11](../../images/backtrace/12.png)





## 总结


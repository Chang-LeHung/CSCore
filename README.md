# CSCore（所有内容都将持续更新...）

本项目专注于计算机系统基础，比如算法与数据结构，网络，操作系统等！！！同时将会包含一些机器学习和人工智能的算法！！！夯实好计算机系统基础，学习什么技术都将变得更加轻松，能够更好的理解各种软件的原理！！！同时欢迎大家提出pr，修正文章当中的错误，或者改进文章，使得文章更好阅读，或者其他能够改进这个项目的提交！！！

<div align="center">
       <p>推荐使用微信公众号阅读，国内网速快，而且渲染公式时github有时候会问题，渲染结果不好，微信公众号阅读体验更好</p>
</div>

---

## 容器（集合）设计与实现及Java集合源码剖析

### 基础容器

**容器设计与实现**主要是学习常见的容器比如`ArrayList`、`LinkedList`、`Deque`和`HashMap`等容器的原理，并且自己动手使用`Java`实现对应的自己的容器（`MyArrayList`、`MyLinkedList`等等）！！！

- 链表设计与Java实现——自己动手写`LinkedList`
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247483691&idx=1&sn=5f730f9ca5f84aa97a13a730ed6d85df&chksm=cf0c9d22f87b14347aed05fa3c0cd313f278618e43aef37cd4fc078fb124c9845b527ba55d3d&token=917021200&lang=zh)
  - [github阅读](./container/01-链表设计与实现.md)
- `LinekdList`源码深度剖析
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247483907&idx=1&sn=6281a11e6ed1917ecb3a10319474193d&chksm=cf0c9e0af87b171c7193949b5b7eb0b8f813b05b3d3b96ea784df86a3dde4286ad03908122da&token=917021200&lang=zh)
  - [github阅读](./container/02-linkedlist源码剖析.md)
- 数组容器(`ArrayList`)设计与`Java`实现——自己动手写`ArrayList`
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247483954&idx=1&sn=0671552fd0d33e0b07eab22f698ffaea&chksm=cf0c9e3bf87b172d1e336d7533a79c536600518c085a4104f30667469992e797ff526c732563&mpshare=1&scene=23&srcid=0706NTX8kMWtQryLVoqdqEYC&sharer_sharetime=1657120181327&sharer_shareid=236a49567847c05f78e6b440ce6dabff#rd)
  - [github阅读](./container/03-array容器设计与实现.md)
- `ArrayList`源码剖析，从扩容原理，到迭代器和fast-fail机制，你想要的这都有！！！
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484031&idx=1&sn=f5b70f87f97a0a21f3fb88bd7343bb25&chksm=cf0c9e76f87b17608f27ff60da5df43d04939e04b4c771b27bc4283c520ddcf15e114d2872de&token=1155116583&lang=zh_CN#rd)
  - [github阅读](./container/04-arraylist源码剖析.md)
- `HashMap`设计原理与实现（上篇）——哈希表的原理，如何从0到1设计一个`HashMap`
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484145&idx=1&sn=362cf64866ace02ac95c0c1a970393e4&chksm=cf0c9ef8f87b17eebb61ea422f58e9e439632783e9faa5a3b2ce55712c1582b140904b60cb17&token=1155116583&lang=zh_CN#rd)
  - [github阅读](./container/05-hashmap设计与实现(上篇).md)
- `HashMap`设计原理与实现（下篇）200行代码带你写自己的`HashMap`！！！
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484235&idx=1&sn=e4fda2cd7520d2d68d7a3c179c8845b3&chksm=cf0c9f42f87b1654e49e21d043fed104ce5fd4839f2eae13cd95c7630fd547e208a1318fd8d3&token=1155116583&lang=zh_CN#rd)
  - [github阅读](./container/06-hashmap设计与实现(下篇).md)
- `HashMap`源码深度剖析，手把手带你分析每一行代码，包会！！！
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484364&idx=1&sn=12c645e61113de9390ee142ea556503b&chksm=cf0c9fc5f87b16d3722d75c93f930a64b1270f8532c115ec62d5e6ddf470aa6f3951e2096050&token=883596793&lang=zh_CN#rd)
  - [github阅读](./container/07-hashmap源码剖析.md)
- `ArrayDeque`（JDK双端队列）源码深度剖析
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484612&idx=1&sn=63a5a21fab640619333d9836a000ea44&chksm=cf0c98cdf87b11db7d63b2d028f0a70ea73e7c84338bce8e7cdf4bee9d5b0c7ec2bf23663233&token=1311889589&lang=zh_CN#rd)
  - [github阅读](./container/08-arraydeque源码剖析.md)
- 深入剖析（JDK）`ArrayQueue` 源码
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484813&idx=1&sn=ace534e0492bbc9f77b9cf488c7c4edc&chksm=cf0c9984f87b109210387f71d9591450ead667611351cd6857b7accb6f1fdc02939e12ad0434&token=969171239&lang=zh_CN#rd)
  - [github阅读](./container/09-arrayqueue源码剖析.md)

---

## 算法

### 动态规划

#### 背包问题

- 庖丁解牛斐波拉契数列和背包问题——详细解析两个问题优化过程（超多问题优化干货，不看血亏），带你从最基本最核心的问题看懂动态规划！！
  
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484109&idx=1&sn=4cd2040eacb04710694282192edeafc4&chksm=cf0c9ec4f87b17d2093a429755c0b3177ddeb39a96d77e210b03fe03f5a76968b7062234594a&token=1155116583&lang=zh_CN#rd)
  - [github阅读](./datastructr&algorithm/01-动态规划.md)
- 你真的懂01背包问题吗？01背包的这几问你能答出来吗？
  
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484416&idx=1&sn=d8aa70bc642c94a127ea67409808980f&chksm=cf0c9809f87b111f2fb092adba83da7e5463a8f5eaa92914ddb975065428a1a80a7d6bc53f3a&token=883596793&lang=zh_CN#rd)
  - [github阅读](./datastructr&algorithm/02-01背包问题.md)
- 面试官：完全背包都不会，是你自己走还是我送你？
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484544&idx=1&sn=c4de17583010430fa519ecd1703bedea&chksm=cf0c9889f87b119fe5621bacf417b163020dcd8a7c0ed63df94de20ba67ae742b4d86e22ae16&token=883596793&lang=zh_CN#rd)
  - [github阅读](./datastructr&algorithm/03完全背包.md)
- 深入剖析多重背包问题（上篇）
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484627&idx=1&sn=ac975cb31ba1af89425558cfe4442258&chksm=cf0c98daf87b11cc4178c55864ba8eeecd741aaa49bde712aca1c5999f7dbbbbd0997beb6cc2&token=1311889589&lang=zh_CN#rd)
  - [github阅读](./datastructr&algorithm/04多重背包v1.md)
- 深入剖析多重背包问题（下篇）
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247484703&idx=1&sn=d0fb3c949b99a803a30a5dd1452d7bce&chksm=cf0c9916f87b10002e32bf8acfa298c1201d33793c80beb538ac099024868373e4c7a5b35a53&token=1311889589&lang=zh_CN#rd)
  - [github阅读](./datastructr&algorithm/04多重背包v2.md)
- 深入浅出零钱兑换问题——背包问题的套壳
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247485777&idx=1&sn=69ffd91b7669704c03127ecb9a18381c&chksm=cf0c9558f87b1c4e3df12503c6d933c5693f05dba9c60df278db1601026d3bd14221bda5ef64&token=1645117295&lang=zh_CN#rd)
  - [github阅读](./datastructr&algorithm/13零钱兑换.md)
- 深入剖析斐波拉契数列
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247485229&idx=1&sn=3d4abd19abfcc42f33de534cdaf4b6dc&chksm=cf0c9b24f87b1232ef3ad1bd3d9fc07467a7e5a003e7e8876fcf5091403d47bd43948a836004&token=1125062744&lang=zh_CN#rd)
  - [github阅读](./datastructr&algorithm/05深入剖析斐波拉契数列.md)

#### 股票问题

- 这种动态规划你见过吗——状态机动态规划之股票问题(上)
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247485286&idx=1&sn=3d0a6a1c2e62ba770d8427c6dd732973&chksm=cf0c9b6ff87b1279d46c775001fd77b8e1437d4001a6c8a1ca8db090eadb4174af1058a1aadf&token=1125062744&lang=zh_CN#rd)
  - [github阅读](./datastructr&algorithm/09状态机动态规划.md)
- 这种动态规划你见过吗——状态机动态规划之股票问题(中)
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247485500&idx=1&sn=f9283ccc6e0c909641eadb9c761f6d1b&chksm=cf0c9435f87b1d23f1361bdd62946d3e6bb79b1415718fd15150bd47215b5ab490510bb51967&token=1092368950&lang=zh_CN#rd)
  - [github阅读](./datastructr&algorithm/10状态机动态规划02.md)
- 这种动态规划你见过吗——状态机动态规划之股票问题(下)
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247485619&idx=1&sn=d9cd4f7abfbedd9cd638e0952acc4826&chksm=cf0c94baf87b1dacd06e2f36ef91a2bc22e05bf8d746727cb39ba2a378a50a07a3fbc8f241c7&token=1092368950&lang=zh_CN#rd)
  - [github阅读](./datastructr&algorithm/11状态机动态规划03.md)
- 状态机动态规划之股票问题总结
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486020&idx=1&sn=d54b40cc6219855b9708d3fd62023a78&chksm=cf0c964df87b1f5bc42a940a72628b0bb3f251811bdee093e2e94438437d214f8c538f37fba7&token=1645117295&lang=zh_CN#rd)
  - [github阅读](./datastructr&algorithm/12状态机动态规划总结.md)

### 数据结构

- 深入剖析前缀、中缀、后缀表达式以及表达式求值

  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247485085&idx=1&sn=27950d2ccfa01419c7b434a676cdd5a3&chksm=cf0c9a94f87b138225ddbf0538c8af6de3a22f4e155329735c46369ec01860837df77e8234b7&token=969171239&lang=zh_CN#rd)

  - [github阅读](./datastructr&algorithm/07表达式求值.md)



## Java

### 并发

- 并发开篇——带你从0到1建立并发知识体系的基石

  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247485015&idx=1&sn=9db029931bf0f3277bfbfc82cbda3824&chksm=cf0c9a5ef87b13487e669579193928572798e83f7d08375b08e9fc305afa08fd4471a8f2a90e&token=969171239&lang=zh_CN#rd)

  - [github阅读](./concurrency/java/01初始java并发.md)
- 并发程序的噩梦——数据竞争

  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247485180&idx=1&sn=601ea1ef37536e4dacf565646ff8e254&chksm=cf0c9af5f87b13e30488d95ae4b46dce714d6fadda6c6b781ab6c6437e992bd36c7a61eb790d&token=969171239&lang=zh_CN#rd)

  - [github阅读](./concurrency/java/02并发的噩梦.md)
- 30行自己写并发工具类(Semaphore, CyclicBarrier, CountDownLatch)是什么体验?
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247485228&idx=1&sn=382890d77927f56f9d884ace679d9a6e&chksm=cf0c9b25f87b123394967465be63156cd805f5c769dc5365b92dc47278a57f4bccc08d102609&token=27677037&lang=zh_CN#rd)

  - [github阅读](./concurrency/java/03自己动手写并发工具类.md)
- 万字长文：从计算机本源深入探寻volatile和Java内存模型
  - [微信公众号阅读](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486127&idx=1&sn=29d6079f6f26bd82633ec611feb3da85&chksm=cf0c96a6f87b1fb006e2f108879a0066aeb14e4bf5a4a9e2a83057a084dd2dfa2c257a813399&token=302443384&lang=zh_CN#rd)

  - [github阅读](./concurrency/java/04volatile.md)

## Python

## 计算机系统基础


---

关注微信公众号，更多精彩内容，实时推送文章！！！

![微信公众号](qrcode2.jpg)

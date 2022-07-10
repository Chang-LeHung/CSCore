# HashMap设计原理与实现（上篇）——哈希表的原理

在此前的[四篇长文](https://github.com/Chang-LeHung/CSCore)当中我们已经实现了我们自己的`ArrayList`和`LinkedList`，并且分析了`ArrayList`和`LinkedList`的`JDK`源代码。 本篇文章主要跟大家介绍我们非常常用的一种数据结构`HashMap`，在本篇文章当中主要介绍他的实现原理，下篇我们自己动手实现我们自己的`HashMap`，让他可以像`JDK`的`HashMap`一样工作。

## HashMap初识

如果你使用过`HashMap`的话，那你肯定很熟悉`HashMap`给我们提供了一个非常方便的功能就是`键值(key, value)`查找。比如我们通过学生的姓名查找分数。

```java
  public static void main(String[] args) {
    HashMap<String, Integer> map = new HashMap<>();
    map.put("学生A", 60);
    map.put("学生B", 70);
    map.put("学生C", 20);
    map.put("学生D", 85);
    map.put("学生E", 99);
    System.out.println("学生B的分数是：" + map.get("学生B"));
  }
```

我们知道`HashMap`给我们提供查询`get函数`功能的时间复杂度为`O(1)`，他在常数级别的时间复杂度就可以查询到结果。那它是如何做到的呢？

我们知道在计算机当中一个最基本也是唯一的，能够实现常数级别的查询的类型就是数组，数组的查询时间复杂度为`O(1)`，我们只需要通过下标就能访问对应的数据。比如我们想访问下标为`6`的数据，就可以这样：

```java
String[] strs = new String[10];
strs[6] = "一无是处的研究僧";
System.out.println(strs[6]);
```

因此我们要想实现`HashMap`给我们提供的`O(1)`级别查询的时间复杂度的话，就必须使用到数组，而在具体的`HashMap`实现当中，比如说`JDK`底层也是采用数组实现的。


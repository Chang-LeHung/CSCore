# 深入理解 Python 拷贝

## 前言

在本篇文章当中主要给大家介绍 python 当中的拷贝问题，话不多说我们直接看代码，你知道下面一些程序片段的输出结果吗？

```python
a = [1, 2, 3, 4]
b = a
print(f"{a = } \t|\t {b = }")
a[0] = 100
print(f"{a = } \t|\t {b = }")
```

```python
a = [1, 2, 3, 4]
b = a.copy()
print(f"{a = } \t|\t {b = }")
a[0] = 100
print(f"{a = } \t|\t {b = }")
```

```python
a = [[1, 2, 3], 2, 3, 4]
b = a.copy()
print(f"{a = } \t|\t {b = }")
a[0][0] = 100
print(f"{a = } \t|\t {b = }")
```

```python
a = [[1, 2, 3], 2, 3, 4]
b = copy.copy(a)
print(f"{a = } \t|\t {b = }")
a[0][0] = 100
print(f"{a = } \t|\t {b = }")
```

```python
a = [[1, 2, 3], 2, 3, 4]
b = copy.deepcopy(a)
print(f"{a = } \t|\t {b = }")
a[0][0] = 100
print(f"{a = } \t|\t {b = }")
```

在本篇文章当中我们将对上面的程序进行详细的分析。

## Python 内部数据在内存上的逻辑分布

首先我们介绍一下一个比较好用的关于数据在内存上的逻辑分布的网站，https://pythontutor.com/visualize.html#mode=display，我们在这个网站上运行第一份代码：

![01copy](../../../vedio/python/01copy.gif)

从上面的输出结果来看 a 和 b 指向的是同一个内存当中的数据对象。

![01](../../../images/python/01.png)

![01](../../../images/python/02.png)
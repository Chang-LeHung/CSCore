# 组合

## 前言

已经好久没有更新了🤣，从今天开始要保证每周的更新频率了（立个flag，应该能够想到打脸会来的很快😂），今天给大家分享一道`LeetCode`算法题，题目不是很困难，但是从这到简单的题目我们可以分析出回溯算法的几个核心要点，以后遇到需要回溯的题目可以应对的思路，知道应该怎么思考，朝什么方向去寻找解决问题的出口！

## [题目](https://leetcode.cn/problems/combinations/)

>给定两个整数 `n` 和 `k`，返回范围 `[1, n]` 中所有可能的 `k` 个数的组合。你可以按 **任何顺序** 返回答案。

例子：

```java
输入：n = 4, k = 2
输出：
[
  [2,4],
  [3,4],
  [2,3],
  [1,2],
  [1,3],
  [1,4],
]
```



## 解法

乍一看这道题好像是一个很直接的问题，我们只需要从[1, n]当中选出k个数据出来，比如说题目当中给出的，我们需要从[1, 4]这个区间取出两个数，那么我们只需要使用两层for循环即可，像下面这样：

```C++
for (int i = 1; i <= n; i++) {
  	// 在这里选择 i 
    for (int j = i + 1; j <= n; j++) {
        // 每一次循环选择 j 
    }
}
```



但是遗憾的是`k`是一个变量，它不是一个定值，如果他是一个定值的话，那么我们就可以使用上面的循环操作去解决这个问题，而且是很高效的。那这个问题我们应该如何解决的？我们思考一下，对于每一个数我们都有两种选择：选择和不选择，也就是是否需要讲这个数据加到集合当中去。

现在我们对上述给定的例子进行求解，每一个数据都有两种选择，选和不选（很多回溯的问题都是可以按照这种思路）具体过程入下图所示:

![](../../images/backtrace/02.png)

上图是例子的解树，其中绿色的节点表示最终的答案，对于每个节点的数据都有两种选择办法，选和不选，因此上面的解决问题的树结构是一个完全二叉树

## 代码实现

### C++实现

```c++
class Solution {
    vector<vector<int>> ans;
public:
    vector<vector<int>> combine(int n, int k) {
      backtrace(n, k, vector<int>());
      return ans;
    }

    void backtrace(int n, int k, vector<int> tmp, int cur=1) {
      if (tmp.size() == k) {
        ans.push_back(tmp);
        return;
      }
      for (int i = cur; i <= n - (k - tmp.size()) + 1 ; ++i) {
        tmp.push_back(i);
        backtrace(n, k, tmp, i + 1);
        tmp.pop_back();
      }
    }
};

```



```c++


#include <vector>

using namespace std;
class Solution {
    vector<vector<int>> ans;
public:
    vector<vector<int>> combine(int n, int k) {
      backtrace(n, k, vector<int>());
      return ans;
    }

    void backtrace(int n, int k, vector<int> tmp, int cur=1) {
      if (tmp.size() == k) {
        ans.push_back(tmp);
        return;
      }
      for (int i = cur; i <= n - (k - tmp.size()) + 1 ; ++i) {
        tmp.push_back(i);
        backtrace(n, k, tmp, i + 1);
        tmp.pop_back();
      }
    }
};
```

### Java实现

```java
class Solution {
  private List<List<Integer>> ans = new ArrayList<>();

  public List<List<Integer>> combine(int n, int k) {

    backTrace(n, k, new ArrayList<>(), 1);
    return ans;
  }

  public void backTrace(int n, int k, List<Integer> path,
                        int idx) {
    if (path.size() == k){
      ans.add(new ArrayList<>(path));
      return;
    } else if ((path.size() + n - idx + 1) < k || idx > n)
      return;
    path.add(idx);
    backTrace(n, k, path, idx + 1);
    path.remove(path.size() - 1);
    backTrace(n, k, path, idx + 1);
  }

}
```

```java
class Solution {
    private List<List<Integer>> res = new ArrayList<>();
    private List<Integer> path = new ArrayList<>();
    public List<List<Integer>> combine(int n, int k) {

        backtrace(n, k, 1);
        return res;
    }

    public void backtrace(int n, int k,
                          int startPosition) {
        if (path.size() == k) {
            res.add(new ArrayList<>(path));
            return;
        }
        for (int i = startPosition; i <= n; i++) {
            path.add(i);
            backtrace(n, k, i + 1);
            path.remove(path.size() - 1);
        }
    }

}
```


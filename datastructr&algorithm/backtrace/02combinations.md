# 组合总和

## 前言

在上篇文章[通过组合问题看透回溯法](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486751&idx=1&sn=8e9cedd729d01ff8867fcb2c085ecbe3&chksm=cf0c9116f87b18002551eefcd773f4762d79d2c06614b304437bfcd382e14afa031d29cc4dcc&mpshare=1&scene=22&srcid=0921yRhazbXlqGuEZagKb0p9&sharer_sharetime=1663737871952&sharer_shareid=236a49567847c05f78e6b440ce6dabff#rd)当中我们通过介绍一个组合问题，仔细地分析了组合问题的回溯过程。我们之后会继续介绍一些比较经典的回溯算法题，帮助深入彻底理解回溯算法的执行过程和原理。

## [组合总和II](https://leetcode.cn/problems/combination-sum-ii/)







## 代码



```c++
class Solution {
    vector<vector<int>> ans;
    vector<int> path;
public:
    vector<vector<int>> combinationSum3(int k, int n) {
      backtrace(k, n, 1, 0);
      return ans;
    }

    void backtrace(int k, int n, int cur, int curSum) {
      if (curSum == n && path.size() == k) {
        ans.push_back(path);
        return;
      }
      if (cur >= 10 || path.size() >= k || n <= curSum || (path.size() + (10 - cur) + 1) < k)
        return;
      for (int i = cur; i <= 10 - (k - path.size()); ++i) {
        path.push_back(i);
        curSum += i;
        backtrace(k, n, i + 1, curSum);
        curSum -= i;
        path.pop_back();
      }
    }
};

```

```c++
class Solution {
    public List<List<Integer>> combinationSum3(int k, int n) {
        List<List<Integer>> res = new ArrayList<>();
        findCombination(k, n, 0, new ArrayList<>(), 1, res);
        return res;
    }

    public void findCombination(int k, int n, int s, List<Integer> path,
                                int startPosition,
                                List<List<Integer>> res) {
        if (s == n && k == path.size()) {
            res.add(new ArrayList<>(path));
            return;
        } else if (s > n || path.size() >= k) return;
        for (int i = startPosition; i <= 10 - (k - path.size()); i++) {
            path.add(i);
            s += i;
            findCombination(k, n, s, path, i + 1, res);
            path.remove(path.size() - 1);
            s -= i;
        }
    }

}
```

```C++
class Solution {
    vector<vector<int>> ans;
    vector<int> path;
public:
    vector<vector<int>> combinationSum3(int k, int n) {
      backtrace(k, n, 1, 0);
      return ans;
    }

    void backtrace(int k, int n, int cur, int curSum) {
      if (curSum == n && path.size() == k) {
        ans.push_back(path);
        return;
      }
      if (cur >= 10 || path.size() > k || n < cur || (path.size() + (10 - cur) + 1) < k)
        return;
      path.push_back(cur);
      curSum += cur;
      backtrace(k, n, cur + 1, curSum);
      curSum -= cur;
      path.pop_back();
      backtrace(k, n, cur + 1, curSum);
    }
};

```


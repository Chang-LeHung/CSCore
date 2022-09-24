# [组合总和 II](https://leetcode.cn/problems/combination-sum-ii/)

## 题目介绍

>给定一个候选人编号的集合 candidates 和一个目标数 target ，找出 candidates 中所有可以使数字和为 target 的组合。
>
>candidates 中的每个数字在每个组合中只能使用 一次 。
>
>注意：解集不能包含重复的组合。 
>

示例：

```java
输入: candidates = [10,1,2,7,6,1,5], target = 8,
输出:
[
[1,1,6],
[1,2,5],
[1,7],
[2,6]
]
```

示例：

```java
输入: candidates = [2,5,2,1,2], target = 5,
输出:
[
[1,2,2],
[5]
]
```

## 问题分析

在这道问题当中我们仍然是从一组数据当中取出数据进行组合，然后得到指定的和，但是与前面的[组合总和](https://mp.weixin.qq.com/s/7A8-rmw0l5Y8c8SnQ5vqwQ)不同的是，在这个问题当中我们可能遇到重复的数字而且每个数字只能够使用一次。这就给我们增加了很大的困难，因为如果存在相同的数据的话我们就又可能产生数据相同的组合，比如在第二个例子当中我们产生的结果`[1, 2, 2]`其中的2就可能来自`candidates`当中不同位置的2，可以是第一个，可以是第三个，也可以是最后一个2。但是在我们的最终答案当中是不允许存在重复的组合的。当然我们可以按照正常的方式遍历，然后将得到的复合要求的结果加入到一个哈希表当中，对得到的结果进行去重处理。但是这样我们的时间和空间开销都会加大很多。

在这个问题当中为了避免产生重复的集合，我们可以首先将这些数据进行排序，然后进行遍历，我们拿一个数据来进行举例子：`[1,  2 1]`，现在我们将这个数据进行排序得到的结果为：`[1, 1, 2]`，那么遍历的树结构如下：

![15](../../images/backtrace/15.png)

上图表示`[1, 1, 2]`的遍历树，每一个数据都有选和不选两种情况，根据这种分析方式可以构造上面的解树，我们对上面的树进行分析我们可以知道，在上面的树当中有一部分子树是有重复的，如下图所示：

![15](../../images/backtrace/16.png)





```C++
class Solution {
    vector<vector<int>> ans;
    vector<int> path;
public:
    vector<vector<int>> combinationSum2(vector<int>& candidates, int target) {
      sort(candidates.begin(), candidates.end());
      backtrace(candidates, target, 0, 0);
      return ans;
    }

    void backtrace(vector<int>& candidates, int target, int curIdx, int curSum) {
      if (curSum == target) {
        ans.push_back(path);
        return;
      } else if (curSum > target || curIdx >= candidates.size()) {
        return;
      }
      for(int i = curIdx; i < candidates.size() && curSum + candidates[i] <= target; ++i) {
        if (i > curIdx && candidates[i] == candidates[i - 1])
          continue;
        path.push_back(candidates[i]);
        backtrace(candidates, target, i + 1, curSum + candidates[i]);
        path.pop_back();
      }
    }
};
```


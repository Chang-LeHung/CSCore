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


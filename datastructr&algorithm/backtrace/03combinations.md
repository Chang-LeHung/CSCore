





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


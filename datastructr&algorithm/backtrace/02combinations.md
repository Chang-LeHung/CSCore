# 组合总和

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


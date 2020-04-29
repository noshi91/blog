#include <vector>

class cumsum {
  std::vector<int> s;

public:
  cumsum(const std::vector<int> &a) : s(a.size() + 1) {
    int n = a.size();
    s[0] = 0;
    for (int i = 0; i < n; ++i) {
      s[i + 1] = s[i] + a[i];
    }
  }

  int fold(int l, int r) const { return s[r] - s[l]; }
};

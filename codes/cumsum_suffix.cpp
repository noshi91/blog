#include <vector>

class cumsum {
  std::vector<int> s;

public:
  cumsum(const std::vector<int> &a) : s(a) {
    int n = a.size();
    s.push_back(0);
    for (int i = n; i > 0; --i) {
      s[i - 1] += s[i];
    }
  }

  int fold(int l, int r) const { return s[l] - s[r]; }
};

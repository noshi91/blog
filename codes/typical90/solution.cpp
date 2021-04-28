#include <algorithm>
#include <cassert>
#include <map>
#include <tuple>
#include <vector>

template <class Cluster>
Cluster global_biased_merge(const int n,
                            std::vector<std::tuple<int, int, Cluster>> edges) {
  struct cap {
    struct node {
      int size;
      std::vector<node *> children;
      node *parent;
      bool dfs;

      node() : size(0), children(), parent(nullptr), dfs(false) {}
    };

    std::vector<node> node_pool;
    std::map<std::pair<node *const, node *const>, Cluster> edge_info;

    cap(const int n, std::vector<std::tuple<int, int, Cluster>> edges)
        : node_pool(n), edge_info() {
      assert(n >= 2);
      assert(edges.size() == n - 1);
      for (auto &[u, v, edge] : edges) {
        assert(0 <= u && u < n);
        assert(0 <= v && v < n);
        assert(u != v);
        edge_info.insert({{ptr(u), ptr(v)}, std::move(edge)});
        ptr(u)->children.push_back(ptr(v));
        ptr(v)->children.push_back(ptr(u));
      }
    }

    node *ptr(const int v) { return &node_pool[v]; }

    Cluster solve() {
      for (int i = 0; i != node_pool.size(); i += 1) {
        node *const v = ptr(i);
        if (v->children.size() == 1) {
          node *const u = v->children[0];
          precalc(u, v);
          return solve_tree(u);
        }
      }
    }

    Cluster get_edge(node *const u, node *const v) {
      const auto itr = edge_info.find({u, v});
      if (itr != edge_info.end()) {
        return std::move(itr->second);
      } else {
        Cluster ret = std::move(edge_info.at({v, u}));
        ret.flip();
        return ret;
      }
    }

    void precalc(node *const v, node *const parent) {
      assert(!v->dfs);
      v->dfs = true;
      auto &list = v->children;
      v->parent = parent;
      for (int i = 0; i != list.size();) {
        node *const c = list[i];
        if (c == parent) {
          list.erase(list.begin() + i);
        } else {
          precalc(c, v);
          v->size += c->size + 1;
          i += 1;
        }
      }
    }

    Cluster solve_tree(node *const root) {
      node *v = root;
      while (!v->children.empty()) {
        node *heavy = v->children[0];
        for (node *const c : v->children) {
          if (c->size > heavy->size) {
            heavy = c;
          }
        }
        v = heavy;
      }
      return solve_path(root, v);
    }

    Cluster solve_path(node *const top, node *const bottom) {
      if (bottom == top) {
        return get_edge(top->parent, top);
      }
      node *v = bottom;
      while (top->size - bottom->size + 1 <
             2 * (top->size - v->parent->size + 1)) {
        v = v->parent;
      }
      Cluster top_half = solve_path(top, v->parent);
      Cluster bottom_half = solve_path(v, bottom);
      if (v->parent->children.size() != 1) {
        const auto &list = v->parent->children;
        struct heap_data {
          Cluster c;
          int size;
          bool operator<(const heap_data &r) const { return size > r.size; }
          void append(heap_data r) {
            c = Cluster::rake(std::move(c), std::move(r.c));
            size += r.size;
          }
        };
        std::vector<heap_data> heap;
        heap.reserve(list.size() - 1);
        for (node *const c : list) {
          if (c != v) {
            heap.push_back({solve_tree(c), c->size + 1});
          }
        }
        std::make_heap(heap.begin(), heap.end());
        while (heap.size() != 1) {
          std::pop_heap(heap.begin(), heap.end());
          heap_data a = std::move(heap.back());
          heap.pop_back();
          std::pop_heap(heap.begin(), heap.end());
          heap.back().append(std::move(a));
          std::push_heap(heap.begin(), heap.end());
        }
        bottom_half =
            Cluster::rake(std::move(heap.front().c), std::move(bottom_half));
      }
      return Cluster::compress(std::move(top_half), std::move(bottom_half));
    }
  };

  cap cp(n, std::move(edges));
  return cp.solve();
}

#include <array>
#include <iostream>

#include <atcoder/convolution>
#include <atcoder/modint>

using modint = atcoder::static_modint<998244353>;

struct poly {
  std::vector<modint> c;

  poly() : c() {}
  poly(modint c0) : c() { c.push_back(c0); }
  poly(std::vector<modint> c_) : c(std::move(c_)) {}

  friend poly operator*(poly l, poly r) {
    return {atcoder::convolution(std::move(l.c), std::move(r.c))};
  }

  poly &operator+=(poly r) {
    if (c.size() < r.c.size()) {
      std::swap(c, r.c);
    }
    for (int i = 0; i != r.c.size(); i += 1) {
      c[i] += r.c[i];
    }
    return *this;
  }

  void shift() { c.insert(c.begin(), modint(0)); }
};

struct isets {
  std::array<std::array<poly, 2>, 2> count;

  isets(int) : count() {
    count[0][0] = poly(1);
    count[0][1] = poly(1);
    count[1][0] = poly(1);
  }

  static isets compress(isets l, isets r) {
    isets ret(0);
    for (int i = 0; i != 2; i += 1) {
      for (int j = 0; j != 2; j += 1) {
        ret.count[i][j] = l.count[i][1] * r.count[1][j];
        ret.count[i][j].shift();
        ret.count[i][j] += l.count[i][0] * r.count[0][j];
      }
    }
    return ret;
  }

  static isets rake(isets child, isets base) {
    for (int i = 0; i != 2; i += 1) {
      child.count[i][1].shift();
      for (int j = 0; j != 2; j += 1) {
        for (int k = 0; k != 2; k += 1) {
          base.count[i][j] += child.count[i][k];
        }
      }
    }
    return base;
  }

  void flip() { std::swap(count[0][1], count[1][0]); }

  poly result() {
    count[0][1].shift();
    count[1][1].shift();
    count[1][0].shift();
    count[1][1].shift();
    poly ret;
    ret += std::move(count[0][0]);
    ret += std::move(count[0][1]);
    ret += std::move(count[1][0]);
    ret += std::move(count[1][1]);
    return ret;
  }
};

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int N;
  std::cin >> N;

  std::vector<std::tuple<int, int, isets>> edges;
  for (int i = 1; i <= N - 1; i += 1) {
    int A, B;
    std::cin >> A >> B;
    A -= 1;
    B -= 1;
    edges.push_back({A, B, isets(0)});
  }

  if (N == 1) {
    std::cout << "0 1\n";
  } else {
    isets res = global_biased_merge(N, std::move(edges));
    std::vector<modint> ans = res.result().c;
    ans.resize(N + 1, modint(0));
    for (int i = 0; i <= N; i += 1) {
      std::cout << ans[i].val() << " \n"[i == N];
    }
  }
  return 0;
}

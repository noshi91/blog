#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

using u64 = std::uint64_t;
using usize = std::size_t;
using std::vector;

template <class T> static constexpr T inf = std::numeric_limits<T>::max();

template <class T> class edge_type {
public:
  usize to;
  T cost;

  bool operator<(const edge_type &r) const { return r.cost < cost; }
};

template <class T> class fibonacci_heap {
  class node_type;
  using node_ptr = node_type *;
  class node_type {
  public:
    node_ptr parent;
    node_ptr child;
    node_ptr left;
    node_ptr right;
    usize rank;
    bool mark;
    T key;

    node_type()
        : parent(nullptr), child(nullptr), left(nullptr), right(nullptr),
          rank(0), mark(false), key(inf<T>) {}
  };

  vector<node_type> nodes;
  node_ptr root;
  vector<node_ptr> table;

public:
  fibonacci_heap(const usize n)
      : nodes(n), root(nullptr),
        table(std::ceil(std::log(n + 1) * 2.08), nullptr) {}

  bool empty() const { return root == nullptr; }

  edge_type<T> pop() {
    edge_type<T> ret = {static_cast<usize>(root - nodes.data()), root->key};

    usize max = 0;
    const auto push = [&](node_ptr v) -> void {
      while (true) {
        node_ptr u = table[v->rank];
        if (u == nullptr) {
          table[v->rank] = v;
          break;
        }
        table[v->rank] = nullptr;
        if (u->key < v->key) {
          std::swap(u, v);
        }
        const node_ptr c = v->child;
        if (c == nullptr) {
          u->left = u;
          u->right = u;
          v->child = u;
        } else {
          u->left = c->left;
          u->right = c;
          c->left->right = u;
          c->left = u;
        }
        u->parent = v;
        v->rank += 1;
      }
      max = std::max(max, v->rank + 1);
    };

    {
      node_ptr v = root->right;
      while (v != root) {
        const node_ptr next = v->right;
        push(v);
        v = next;
      }
    }
    if (root->child != nullptr) {
      node_ptr v = root->child;
      do {
        const node_ptr next = v->right;
        v->mark = false;
        push(v);
        v = next;
      } while (v != root->child);
    }
    root = nullptr;

    for (usize i = 0; i != max; i += 1) {
      const node_ptr v = table[i];
      if (v == nullptr) {
        continue;
      }
      table[i] = nullptr;
      v->parent = nullptr;
      if (root == nullptr) {
        root = v;
        v->left = v;
        v->right = v;
      } else {
        v->left = root->left;
        v->right = root;
        root->left->right = v;
        root->left = v;
        if (root->key > v->key) {
          root = v;
        }
      }
    }

    return ret;
  }

  void update_key(const usize v_, const T key) {
    node_ptr v = &nodes[v_];

    if (v->key <= key) {
      return;
    }
    v->key = key;

    if (v->left == nullptr) {
      if (root == nullptr) {
        v->left = v;
        v->right = v;
        root = v;
      } else {
        v->left = root->left;
        v->right = root;
        root->left->right = v;
        root->left = v;
        if (key < root->key) {
          root = v;
        }
      }
      return;
    }

    if (v->parent == nullptr) {
      if (key < root->key) {
        root = v;
      }
      return;
    } else {
      if (v->parent->key <= key) {
        return;
      }
    }

    while (true) {
      const node_ptr p = v->parent;
      v->left->right = v->right;
      v->right->left = v->left;
      v->parent = nullptr;
      p->rank -= 1;
      if (p->child == v) {
        if (p->rank == 0) {
          p->child = nullptr;
        } else {
          p->child = v->right;
        }
      }

      v->left = root->left;
      v->right = root;
      root->left->right = v;
      root->left = v;
      v->mark = false;

      v = p;
      if (v->parent == nullptr) {
        break;
      }
      if (!v->mark) {
        v->mark = true;
        break;
      }
    }
    if (root->key > key) {
      root = &nodes[v_];
    }
  }
};

template <class T>
vector<T> dijkstra_fibonacci(const vector<vector<edge_type<T>>> &graph,
                             const usize source) {
  vector<T> ret(graph.size(), inf<T>);
  fibonacci_heap<T> heap(graph.size());
  heap.update_key(source, 0);
  while (!heap.empty()) {
    const auto top = heap.pop();
    ret[top.to] = top.cost;
    for (const auto &edge : graph[top.to]) {
      heap.update_key(edge.to, top.cost + edge.cost);
    }
  }
  return ret;
}

template <class T>
vector<T> dijkstra_binary(const vector<vector<edge_type<T>>> &graph,
                          const usize source) {
  vector<T> ret(graph.size(), inf<T>);
  std::priority_queue<edge_type<T>> que;
  const auto push = [&](const usize v, const T c) -> void {
    if (ret[v] > c) {
      ret[v] = c;
      que.push({v, c});
    }
  };
  push(source, 0);
  while (!que.empty()) {
    const usize v = que.top().to;
    const T c = que.top().cost;
    que.pop();
    if (ret[v] < c) {
      continue;
    }
    for (const auto &e : graph[v]) {
      push(e.to, c + e.cost);
    }
  }

  return ret;
}

template <class T>
vector<T> dijkstra_naive(const vector<vector<edge_type<T>>> &graph,
                         const usize source) {
  const usize n = graph.size();
  vector<T> dist(n, inf<T>);
  vector<bool> visited(n, false);
  dist[source] = 0;
  while (true) {
    usize mindex = n;
    for (usize i = 0; i != n; i += 1) {
      if (!visited[i] && (mindex == n || dist[mindex] > dist[i])) {
        mindex = i;
      }
    }
    if (mindex == n) {
      break;
    }
    visited[mindex] = true;
    for (const auto &e : graph[mindex]) {
      dist[e.to] = std::min(dist[e.to], dist[mindex] + e.cost);
    }
  }

  return dist;
}

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <numeric>
#include <random>

template <class T> struct edge_info {
  usize u, v;
  T cost;
};

void run(const usize n, const usize deg, const bool run_naive) {
  vector<edge_info<u64>> edges;
  {
    u64 next = u64(1) << 32;
    edges.push_back({0, 1, 1});
    for (usize v = 2; v != n; v += 1) {
      edges.push_back({0, v, next});
      next -= 1;
    }
    for (usize u = 1; u + 1 != n; u += 1) {
      edges.push_back({u, u + 1, 1});
      for (usize v = u + 2; v != n && v != u + deg + 1; v += 1) {
        edges.push_back({u, v, next - u});
        next -= 1;
      }
    }
  }

  usize source;
  vector<vector<edge_type<u64>>> graph(n);
  {
    std::mt19937_64 rnd(91);
    std::shuffle(edges.begin(), edges.end(), rnd);
    vector<usize> map(n);
    std::iota(map.begin(), map.end(), usize(0));
    std::shuffle(map.begin(), map.end(), rnd);
    for (const auto &e : edges) {
      graph[map[e.u]].push_back({map[e.v], e.cost});
    }
    source = map[0];
  }

  std::cout << "V = " << n << ", E = " << edges.size() << "\n";

  const auto ans = dijkstra_fibonacci(graph, source);

  const auto time_median = [&](auto f) -> int {
    std::vector<int> times;
    for (usize i = 0; i != 15; i += 1) {
      const auto start = std::chrono::system_clock::now();
      const auto res = f();
      const auto end = std::chrono::system_clock::now();
      const auto span =
          std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
              .count();
      times.push_back(span);
      assert(res == ans);
    }
    std::sort(times.begin(), times.end());
    return times[times.size() / 2];
  };
  {
    const int fib =
        time_median([&]() { return dijkstra_fibonacci(graph, source); });
    std::cout << fib << " ";
  }
  {
    const int bin =
        time_median([&]() { return dijkstra_binary(graph, source); });
    std::cout << bin << " ";
  }
  if (run_naive) {
    const int naive =
        time_median([&]() { return dijkstra_naive(graph, source); });
    std::cout << naive << " ";
  }
  std::cout << "\n";
}

int main() {

  run(1500000, 3, false);
  run(500000, 20, false);
  run(5000, 5000, true);

  return 0;
}

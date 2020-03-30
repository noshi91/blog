#include <cstdint>

class modint {
  using u64 = std::uint_fast64_t;

public:
  static u64 mod;

  u64 v;

  modint(const u64 x = 0) : v(x % mod) {}
  modint operator+(const modint rhs) const { return modint(*this) += rhs; }
  modint operator-(const modint rhs) const { return modint(*this) -= rhs; }
  modint operator*(const modint rhs) const { return modint(*this) *= rhs; }
  modint operator/(const modint rhs) const { return modint(*this) /= rhs; }
  modint &operator+=(const modint rhs) {
    v += rhs.v;
    if (v >= mod)
      v -= mod;
    return *this;
  }
  modint &operator-=(const modint rhs) {
    if (v < rhs.v)
      v += mod;
    v -= rhs.v;
    return *this;
  }
  modint &operator*=(const modint rhs) {
    v = v * rhs.v % mod;
    return *this;
  }
  modint &operator/=(modint rhs) {
    u64 exp = mod - 2;
    while (exp != 0) {
      if (exp % 2 != 0)
        *this *= rhs;
      rhs *= rhs;
      exp /= 2;
    }
    return *this;
  }
};
typename modint::u64 modint::mod = 0;

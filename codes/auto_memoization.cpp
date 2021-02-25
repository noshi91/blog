#include <cstddef>
#include <map>
#include <tuple>
#include <type_traits>
#include <utility>

template <class F> class memoized_impl {
  template <class> struct get_sig {};

  template <class T, class R, class S, class... Args>
  struct get_sig<R (T::*)(S, Args...) const> {
    using return_type = R;
    using args = std::tuple<std::decay_t<Args>...>;
  };

  using signature =
      get_sig<decltype(&F::template operator()<memoized_impl<F> &>)>;
  using R = typename signature::return_type;
  using ArgsTuple = typename signature::args;

  F func;
  std::map<ArgsTuple, R> memo;

  memoized_impl(const F &func_) : func(func_), memo() {}

  template <std::size_t... I>
  R apply(const ArgsTuple &args_tuple, std::index_sequence<I...>) {
    return func(*this, std::get<I>(args_tuple)...);
  }

public:
  template <class... Args> R operator()(Args &&... args) {
    ArgsTuple args_tuple(std::forward<Args>(args)...);
    const auto itr = memo.find(args_tuple);
    if (itr != memo.end()) {
      return itr->second;
    }
    R res =
        apply(args_tuple,
              std::make_index_sequence<std::tuple_size<ArgsTuple>::value>());
    memo.emplace(std::move(args_tuple), res);
    return res;
  }

  template <class F_> friend auto memoized(const F_ &);
};

template <class F_> auto memoized(const F_ &func) {
  return memoized_impl<F_>(func);
}

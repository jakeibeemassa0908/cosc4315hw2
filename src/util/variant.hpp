#ifndef COSC4315HW2_SRC_UTIL_VARIANT_HPP_
#define COSC4315HW2_SRC_UTIL_VARIANT_HPP_

#include <type_traits>

#include <mpark/variant.hpp>

namespace Util {
template <class...>
struct disjunction : std::false_type {};
template <class T>
struct disjunction<T> : T {};
template <class T, class... Ts>
struct disjunction<T, Ts...>
    : std::conditional<bool(T::value), T, disjunction<Ts...>>::type {};

template <class T, class Variant>
struct is_variant_member;
template <class T, class... VTs>
struct is_variant_member<T, mpark::variant<VTs...>>
    : public disjunction<std::is_same<T, VTs>...> {};

template <class... Funs>
struct __visitor;

template <class Fun, class... Rest>
struct __visitor<Fun, Rest...> : Fun, __visitor<Rest...> {
  __visitor(Fun fun, Rest... rest) : Fun(fun), __visitor<Rest...>(rest...) {}

  using Fun::operator();
  using __visitor<Rest...>::operator();
};

template <class Fun>
struct __visitor<Fun> : Fun {
  __visitor(Fun fun) : Fun(fun) {}

  using Fun::operator();
};

template <class... Funs>
auto make_visitor(Funs... funs) {
  return __visitor<Funs...>(funs...);
}
}  // namespace Util

#endif

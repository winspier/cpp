#pragma once

template <bool B>
struct conditional_impl;

template <>
struct conditional_impl<true> {
  template <typename T, typename F>
  using type = T;
};

template <>
struct conditional_impl<false> {
  template <typename T, typename F>
  using type = F;
};

template <bool B, typename T, typename F>
using conditional = typename conditional_impl<B>::template type<T, F>;

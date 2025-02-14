#pragma once

#include "helpers.h"

namespace details {
template <size_t N, typename Variant>
constexpr decltype(auto) get_impl(Variant&& v) {
  if (v.index() != N) {
    throw bad_variant_access("invalid index in get");
  }
  return std::forward<Variant>(v).data_.template get<N>();
}
} // namespace details

template <typename T, typename... Types>
  requires details::IsExactlyOneOf<T, Types...>
constexpr bool holds_alternative(const variant<Types...>& v) noexcept {
  return v.index() == details::IndexOf_v<T, Types...>;
}

template <size_t N, typename... Types>
constexpr variant_alternative_t<N, variant<Types...>>& get(variant<Types...>& v) {
  return details::get_impl<N>(v);
}

template <size_t N, typename... Types>
constexpr const variant_alternative_t<N, variant<Types...>>& get(const variant<Types...>& v) {
  return details::get_impl<N>(v);
}

template <size_t N, typename... Types>
constexpr variant_alternative_t<N, variant<Types...>>&& get(variant<Types...>&& v) {
  return details::get_impl<N>(std::move(v));
}

template <size_t N, typename... Types>
constexpr const variant_alternative_t<N, variant<Types...>>&& get(const variant<Types...>&& v) {
  return details::get_impl<N>(std::move(v));
}

template <typename Tp, typename... Types>
  requires details::IsExactlyOneOf<Tp, Types...>
constexpr Tp& get(variant<Types...>& v) {
  return get<details::IndexOf_v<Tp, Types...>>(v);
}

template <typename Tp, typename... Types>
  requires details::IsExactlyOneOf<Tp, Types...>
constexpr Tp&& get(variant<Types...>&& v) {
  return get<details::IndexOf_v<Tp, Types...>>(std::move(v));
}

template <typename Tp, typename... Types>
  requires details::IsExactlyOneOf<Tp, Types...>
constexpr const Tp& get(const variant<Types...>& v) {
  return get<details::IndexOf_v<Tp, Types...>>(std::move(v));
}

template <typename Tp, typename... Types>
  requires details::IsExactlyOneOf<Tp, Types...>
constexpr const Tp&& get(const variant<Types...>&& v) {
  return get<details::IndexOf_v<Tp, Types...>>(std::move(v));
}

template <size_t N, typename... Types>
constexpr variant_alternative_t<N, variant<Types...>>* get_if(variant<Types...>* ptr) noexcept {
  if (ptr && ptr->index() == N) {
    return std::addressof(get<N>(*ptr));
  }
  return nullptr;
}

template <size_t N, typename... Types>
constexpr const variant_alternative_t<N, variant<Types...>>* get_if(const variant<Types...>* ptr) noexcept {
  if (ptr && ptr->index() == N) {
    return std::addressof(get<N>(*ptr));
  }
  return nullptr;
}

template <typename T, typename... Types>
  requires details::IsExactlyOneOf<T, Types...>
constexpr T* get_if(variant<Types...>* ptr) noexcept {
  return get_if<details::IndexOf_v<T, Types...>>(ptr);
}

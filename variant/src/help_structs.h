#pragma once
#include "helpers.h"

struct monostate {};

class bad_variant_access : public std::exception {
public:
  bad_variant_access() noexcept {}

  const char* what() const noexcept override {
    return reason_;
  }

  bad_variant_access(const char* reason) noexcept
      : reason_(reason) {}

private:
  const char* reason_ = "bad variant access";
};

template <typename... Types>
class variant;

template <typename V>
struct variant_size;

template <typename V>
struct variant_size<const V> : variant_size<V> {};

template <typename... Types>
struct variant_size<variant<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {};

template <typename V>
inline constexpr size_t variant_size_v = variant_size<V>::value;

template <typename... Types>
inline constexpr size_t variant_size_v<variant<Types...>> = sizeof...(Types);

template <typename... Types>
inline constexpr size_t variant_size_v<const variant<Types...>> = sizeof...(Types);

template <size_t N, typename V>
struct variant_alternative;

template <size_t N, typename... Types>
  requires (N < sizeof...(Types))
struct variant_alternative<N, variant<Types...>> {
  using type = details::nth_type_t<N, Types...>;
};

template <size_t N, typename V>
using variant_alternative_t = typename variant_alternative<N, V>::type;

template <size_t N, typename V>
struct variant_alternative<N, const V> {
  using type = const variant_alternative_t<N, V>;
};

struct in_place_t {
  explicit in_place_t() = default;
};

template <typename T>
struct in_place_type_t {
  explicit in_place_type_t() = default;
};

template <size_t N>
struct in_place_index_t {
  explicit in_place_index_t() = default;
};

inline constexpr in_place_t in_place{};

template <typename T>
inline constexpr in_place_type_t<T> in_place_type{};

template <size_t N>
inline constexpr in_place_index_t<N> in_place_index{};

template <size_t N, typename Variant, typename... Args>
concept is_constructible =
    (N < variant_size_v<Variant> && std::is_constructible_v<variant_alternative_t<N, Variant>, Args&&...>);

template <typename... Types>
constexpr variant<Types...>& as_variant(variant<Types...>& v) noexcept {
  return v;
}

template <typename... Types>
constexpr const variant<Types...>& as_variant(const variant<Types...>& v) noexcept {
  return v;
}

template <typename... Types>
constexpr variant<Types...>&& as_variant(variant<Types...>&& v) noexcept {
  return std::move(v);
}

template <typename... Types>
constexpr const variant<Types...>&& as_variant(const variant<Types...>&& v) noexcept {
  return std::move(v);
}

template <size_t N, typename Variant>
using variant_type = decltype(get<N>(as_variant(std::declval<Variant>())));

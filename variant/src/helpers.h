#pragma once

#include <string>
#include <type_traits>

inline constexpr std::size_t variant_npos = static_cast<size_t>(-1);

template <typename... Types>
class variant;

namespace details {
template <typename T, typename... Types>
inline constexpr bool is_multi_same = (std::is_same_v<T, Types> && ...);

template <size_t N, typename... Types>
struct nth_type;

template <typename T, typename... Rest>
struct nth_type<0, T, Rest...> {
  using type = T;
};

template <size_t N, typename T1, typename T2, typename... Rest>
  requires (N >= 1)
struct nth_type<N, T1, T2, Rest...> {
  using type = typename nth_type<N - 1, T2, Rest...>::type;
};

template <size_t N, typename... Types>
using nth_type_t = typename nth_type<N, Types...>::type;

template <typename Variant>
constexpr size_t variant_index(Variant&& v) {
  return v.index();
}

template <typename... Types, typename T>
constexpr decltype(auto) variant_cast(T&& t) {
  if constexpr (std::is_lvalue_reference_v<T>) {
    if constexpr (std::is_const_v<std::remove_reference_t<T>>) {
      return static_cast<const variant<Types...>&>(t);
    } else {
      return static_cast<variant<Types...>&>(t);
    }
  } else {
    return static_cast<variant<Types...>&&>(t);
  }
}

template <typename T, typename... Types>
concept IsOneOf = (std::is_same_v<T, Types> || ...);

template <typename T, typename... Types>
concept IsExactlyOneOf = (std::is_same_v<T, Types> + ...) == 1;

template <typename... Types>
struct IndexOf;

template <typename T, typename First, typename... Rest>
  requires (!std::is_same_v<T, First> && IsOneOf<T, Rest...>)
struct IndexOf<T, First, Rest...> {
  static constexpr size_t value = 1 + IndexOf<T, Rest...>::value;
};

template <typename T, typename First, typename... Rest>
  requires std::is_same_v<T, First>
struct IndexOf<T, First, Rest...> {
  static constexpr size_t value = 0;
};

template <typename T, typename First, typename... Rest>
  requires (!IsOneOf<T, First, Rest...>)
struct IndexOf<T, First, Rest...> {
  static constexpr size_t value = static_cast<size_t>(-1);
};

template <typename... Types>
inline constexpr size_t IndexOf_v = IndexOf<Types...>::value;

template <typename... Types>
struct first_type;

template <typename First, typename... Rest>
struct first_type<First, Rest...> {
  using type = First;
};

template <typename First>
struct first_type<First> {
  using type = First;
};

template <typename... Types>
using first_type_t = typename first_type<Types...>::type;

template <typename T, size_t Id, bool nothrow>
struct nice_data {
  using type = T;
  static constexpr size_t index = Id;
  static constexpr bool is_nothrow = nothrow;
};

template <typename T, typename U, size_t Idx, typename = void>
struct nice_ctor_alternative {
  constexpr void operator()() = delete;
};

template <typename T>
struct arr_check_type {
  T data[1];
};

template <typename To, typename From, size_t Idx>
struct nice_ctor_alternative<To, From, Idx, std::void_t<decltype(arr_check_type<To>{{std::declval<From>()}})>> {
  constexpr nice_data<To, Idx, noexcept(arr_check_type<To>{{std::declval<From>()}})> operator()(To);
};

template <typename... Types>
struct find_nice_ctor;

template <typename From, typename... Types, size_t... Idx>
struct find_nice_ctor<From, std::tuple<Types...>, std::index_sequence<Idx...>>
    : nice_ctor_alternative<Types, From, Idx>... {
  using nice_ctor_alternative<Types, From, Idx>::operator()...;
};

template <typename From, typename... Types>
using make_find_nice_ctor = find_nice_ctor<From, std::tuple<Types...>, std::make_index_sequence<sizeof...(Types)>>;

template <typename From, typename... Types>
concept is_nice_constructible =
    !std::is_same_v<void, decltype(make_find_nice_ctor<From, Types...>{}(std::declval<From>()))>;

template <typename T, typename... Types>
inline constexpr bool is_nothrow_nice_ctor =
    decltype(make_find_nice_ctor<T, Types...>{}(std::declval<T>()))::is_nothrow;

struct empty {
  static constexpr size_t index = variant_npos;
};

template <typename T, typename... Types>
struct nice_index {
  static constexpr size_t index = variant_npos;
};

template <typename T, typename... Types>
  requires is_nice_constructible<T, Types...>
struct nice_index<T, Types...> {
  static constexpr size_t index = decltype(make_find_nice_ctor<T, Types...>{}(std::declval<T>()))::index;
};

template <typename T, typename... Types>
inline constexpr size_t nice_index_v = nice_index<T, Types...>::index;

template <typename T, typename... Types>
using nice_ctor_type = typename decltype(make_find_nice_ctor<T, Types...>{}(std::declval<T>()))::type;
} // namespace details

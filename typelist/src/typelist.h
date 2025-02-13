#pragma once

#include "conditional.h"

#include <functional>
#include <tuple>
#include <type_traits>

namespace tl {

template <typename... Types>
struct type_list {};

// Concatenate an arbitrary number of typelists
template <typename Type>
struct identity {
  using type = Type;
};

template <
    template <typename...>
    typename List1,
    template <typename...>
    typename List2,
    typename... Types1,
    typename... Types2>
identity<List1<Types1..., Types2...>> operator+(identity<List1<Types1...>>, identity<List2<Types2...>>);

template <typename... Lists>
using concat = typename decltype((identity<Lists>{} + ... + identity<type_list<>>{}))::type;

// Contains
template <typename Type, typename List>
struct contains_impl;

template <typename Type, template <typename...> typename List, typename... Types>
struct contains_impl<Type, List<Types...>> {
  static constexpr bool value = (std::is_same_v<Type, Types> || ...);
};

template <typename Type, typename List>
inline constexpr bool contains = contains_impl<Type, List>::value;

// Transform
template <template <typename...> typename F, typename List>
struct transform_impl;

template <template <typename...> typename F, template <typename...> typename List, typename... Types>
struct transform_impl<F, List<Types...>> {
  using type = List<F<Types>...>;
};

template <template <typename...> typename F, typename List>
using transform = typename transform_impl<F, List>::type;

// Flip and Swap
template <typename List>
struct swap_impl;

template <template <typename...> typename List, typename A, typename B>
struct swap_impl<List<A, B>> {
  using value = List<B, A>;
};

template <typename T>
using swap = typename swap_impl<T>::value;

template <typename List>
using flip = transform<swap, List>;

// Index
template <std::size_t N>
using index = std::integral_constant<std::size_t, N>;

// Count
template <typename List>
struct count_impl;

template <template <typename...> typename List_type, typename... Types>
struct count_impl<List_type<Types...>> {
  static constexpr std::size_t value = sizeof...(Types);
};

template <typename List>
inline constexpr std::size_t count = count_impl<List>::value;

// Enumerate
template <typename List, typename Seq>
struct enumerate_impl;

template <template <typename...> typename List_type, typename... Types, std::size_t... Idxs>
struct enumerate_impl<List_type<Types...>, std::index_sequence<Idxs...>> {
  using type = List_type<List_type<Types, index<Idxs>>...>;
};

template <typename List>
using enumerate = typename enumerate_impl<List, std::make_index_sequence<count<List>>>::type;

// Second
template <typename List>
struct second_impl;

template <template <typename...> typename List_type, typename First, typename Second, typename... Rest>
struct second_impl<List_type<First, Second, Rest...>> {
  using type = Second;
};

template <typename List>
using second = typename second_impl<List>::type;

// First
template <typename List>
struct first_impl {
  using type = List;
};

template <template <typename...> typename List_type, typename First, typename... Rest>
struct first_impl<List_type<First, Rest...>> {
  using type = First;
};

template <typename List>
using first = typename first_impl<List>::type;

// Map Find
template <typename... Bases>
struct inherit : Bases... {};

template <typename Key, typename Map>
struct map_find_impl;

template <typename Key, template <typename...> typename List_type, typename... Lists>
struct map_find_impl<Key, List_type<Lists...>> {
  template <typename... Rest>
  static List_type<Key, Rest...> f(List_type<Key, Rest...>*);

  using derived = inherit<Lists...>;

  using type = decltype(f(static_cast<derived*>(nullptr)));
};

template <typename Key, typename Map>
using map_find = typename map_find_impl<Key, Map>::type;

template <typename Type, typename List>
inline constexpr std::size_t index_of_unique = second<map_find<Type, enumerate<List>>>::value;

// Flatten
template <typename List>
struct flatten_impl;

template <typename Type>
struct flatten_impl {
  using type = type_list<Type>;
};

template <typename List>
using flatten = typename flatten_impl<List>::type;

template <template <typename...> typename List_type, typename... Types>
struct flatten_impl<List_type<Types...>> {
  using type = concat<List_type<>, flatten<Types>...>;
};

// Filter
template <template <typename...> typename Pred, typename List>
struct filter_impl;

template <template <typename...> typename Pred, typename List>
using filter = typename filter_impl<Pred, List>::type;

template <template <typename...> typename Pred, template <typename...> typename List, typename... Types>
struct filter_impl<Pred, List<Types...>> {
  template <typename Type>
  using single = conditional<Pred<Type>::value, List<Type>, List<>>;

  using type = concat<single<Types>...>;
};

template <template <typename...> typename Pred, template <typename...> typename List>
struct filter_impl<Pred, List<>> {
  using type = List<>;
};

// Comparator for enumerated
template <std::size_t N>
struct enumerated_cmp {
  template <typename T>
  struct less {
    static constexpr bool value = second<T>::value < N;
  };

  template <typename T>
  struct greater_equal {
    static constexpr bool value = second<T>::value >= N;
  };
};

// Slices
template <typename List, std::size_t N>
using slice = transform<first, filter<enumerated_cmp<N>::template less, enumerate<List>>>;

template <typename List, std::size_t N>
using slice_from = transform<first, filter<enumerated_cmp<N>::template greater_equal, enumerate<List>>>;

// Merge sorted lists
template <typename List1, typename List2, template <typename...> typename Compare>
struct merge_sorted_impl {
  using type = concat<List1, List2>;
};

template <bool, typename List1, typename List2, template <typename...> typename Compare>
struct merge_sorted_ordered_concat;

template <typename List1, typename List2, template <typename...> typename Compare>
using merge_sorted = typename merge_sorted_impl<List1, List2, Compare>::type;

template <
    template <typename...>
    typename List_type,
    typename List1_head,
    typename... List1_rest,
    typename List2_head,
    typename... List2_rest,
    template <typename...>
    typename Compare>
struct merge_sorted_impl<List_type<List1_head, List1_rest...>, List_type<List2_head, List2_rest...>, Compare> {
  using type = typename merge_sorted_ordered_concat<
      Compare<List1_head, List2_head>::value,
      List_type<List1_head, List1_rest...>,
      List_type<List2_head, List2_rest...>,
      Compare>::type;
};

template <
    template <typename...>
    typename List_type,
    typename Head,
    typename... Rest,
    typename List2,
    template <typename...>
    typename Compare>
struct merge_sorted_ordered_concat<true, List_type<Head, Rest...>, List2, Compare> {
  using type = concat<List_type<Head>, merge_sorted<List_type<Rest...>, List2, Compare>>;
};

template <
    template <typename...>
    typename List_type,
    typename Head,
    typename... Rest,
    typename List2,
    template <typename...>
    typename Compare>
struct merge_sorted_ordered_concat<false, List2, List_type<Head, Rest...>, Compare> {
  using type = concat<List_type<Head>, merge_sorted<List_type<Rest...>, List2, Compare>>;
};

// Merge sort
template <typename List, template <typename...> typename Compare>
struct merge_sort_impl;

template <template <typename...> typename List, template <typename...> typename Compare>
struct merge_sort_impl<List<>, Compare> {
  using type = List<>;
};

template <template <typename...> typename List, typename Type, template <typename...> typename Compare>
struct merge_sort_impl<List<Type>, Compare> {
  using type = List<Type>;
};

template <
    template <typename...>
    typename List,
    typename H,
    typename H2,
    typename... Rest,
    template <typename...>
    typename Compare>
struct merge_sort_impl<List<H, H2, Rest...>, Compare> {
  using list = List<H, H2, Rest...>;
  static constexpr std::size_t mid = count<list> / 2;

  using type = merge_sorted<
      typename merge_sort_impl<slice<list, mid>, Compare>::type,
      typename merge_sort_impl<slice_from<list, mid>, Compare>::type,
      Compare>;
};

template <typename List, template <typename...> typename Compare>
using merge_sort = typename merge_sort_impl<List, Compare>::type;

} // namespace tl

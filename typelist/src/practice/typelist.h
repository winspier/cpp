#pragma once

#include "conditional.h"

#include <tuple>
#include <type_traits>

//contains<Type, List> — возвращает true, если List содержит Type, иначе false;
//flip<List> — принимает список пар типов (пара — тайплист размера 2), возвращает список, в котором у каждой пары элементы переставлены местами;
//index_of_unique<Type, List> — возвращает индекс единственного вхождения Type в List;
//flatten<List> — рекурсивно разворачивает все вложенные тайплисты в один одномерный;
//merge_sort<List, Compare> — возвращает копию List, но с элементами, отсортированными компаратором Compare (он принимает два типа, и возвращает true, если первый должен идти до второго).

namespace tl {

// Type List
template <typename... Types>
struct type_list
{};

// Transform
template <template <typename...> typename F, typename List>
struct transform_impl;

template <template <typename...> typename F, typename... Types>
struct transform_impl<F, type_list<Types...>>
{
    using type = type_list<F<Types>...>;
};

template <template <typename...> typename F, typename List>
using transform = transform_impl<F, List>::type;

// Apply
template <template <typename...> typename F, typename List>
struct apply_impl;

template <template <typename...> typename F, template <typename...> typename List, typename... Types>
struct apply_impl<F, List<Types...>>
{
    using type = F<Types...>;
};

template <template <typename...> typename F, typename List>
using apply = apply_impl<F, List>::type;

// Push Back

template <typename Type, typename List>
struct push_back_impl;

template <typename Type, typename... Types>
struct push_back_impl<Type, type_list<Types...>>
{
    using type = type_list<Types..., Type>;
};

template <typename Type, typename List>
using push_back = push_back_impl<Type, List>::type;

// Concatenate
template <typename List1, typename List2>
struct concat2_impl;

template <typename... Types1, typename... Types2>
struct concat2_impl<type_list<Types1...>, type_list<Types2...>>
{
    using type = type_list<Types1..., Types2...>;
};

template <typename List1, typename List2>
using concat2 = concat2_impl<List1, List2>::type;

// Push back (alt)
template <typename Type, typename List>
using push_back_alt = concat2<List, type_list<Type>>;

// Push front
template <typename Type, typename List>
using push_front = concat2<type_list<Type>, List>;

// Pop front
template <typename List>
struct pop_front_impl;

template <typename Head, typename... Types>
struct pop_front_impl<type_list<Head, Types...>>
{
    using type = type_list<Types...>;
};

template <typename List>
using pop_front = pop_front_impl<List>::type;

// Replace front
template <typename Type, typename List>
using replace_front = push_front<Type, pop_front<List>>;

// Front
template <typename List>
struct front_impl;

template <typename Head, typename... Types>
struct front_impl<type_list<Head, Types...>>
{
    using type = Head;
};

template <typename List>
using front = front_impl<List>::type;

// Reverse
template <typename List>
struct reverse_impl;

template <typename List>
using reverse = reverse_impl<List>::type;

template <typename List>
struct reverse_impl
{
    using type = push_back<front<List>, reverse<pop_front<List>>>;
};

template <>
struct reverse_impl<type_list<>>
{
    using type = type_list<>;
};

// Pop back
template <typename List>
using pop_back = reverse<pop_front<reverse<List>>>;

// Concatenate an arbitrary number of typelists
template <typename... Lists>
struct concat_impl;

template <typename... Lists>
using concat = concat_impl<Lists...>::type;

template <>
struct concat_impl<>
{
    using type = type_list<>;
};

template <typename List>
struct concat_impl<List>
{
    using type = List;
};

template <typename List1, typename List2, typename... RestLists>
struct concat_impl<List1, List2, RestLists...>
{
    using type = concat<concat2<List1, List2>, RestLists...>;
};

// Concatenate an arbitrary number of typelists

template <typename Type>
struct identity
{
    using type = Type;
};

template <template <typename...> typename List1, template <typename...> typename List2, typename... Types1, typename... Types2>
identity<List1<Types1..., Types2...>> operator+(identity<List1<Types1...>>, identity<List2<Types2...>>);

template <typename... Lists>
using concat_fast = decltype((identity<Lists>{} + ... + identity<type_list<>>{}))::type;

// Filter
template <template <typename...> typename Pred, typename List>
struct filter_impl;

template <template <typename...> typename Pred, typename List>
using filter = filter_impl<Pred, List>::type;

template <template <typename...> typename Pred, typename... Types>
struct filter_impl<Pred, type_list<Types...>>
{
    template <typename Type>
    using single = conditional<Pred<Type>::value, type_list<Type>, type_list<>>;

    using type = concat<single<Types>...>;
};

template <template <typename...> typename Pred>
struct filter_impl<Pred, type_list<>>
{
    using type = type_list<>;
};

// Get nth element
template <std::size_t N, typename List>
struct get_nth_impl;

template <std::size_t N, typename List>
using get_nth = get_nth_impl<N, List>::type;

template <typename Head, typename... Rest>
struct get_nth_impl<0, type_list<Head, Rest...>>
{
    using type = Head;
};

template <std::size_t N, typename List>
struct get_nth_impl
{
    using type = get_nth<N - 1, pop_front<List>>;
};

// Index
template <std::size_t N>
using index = std::integral_constant<std::size_t, N>;

// Map Find
template <typename... Bases>
struct inherit : Bases...
{};

template <typename Key, typename Map>
struct map_find_impl;

template <typename Key, typename... Lists>
struct map_find_impl<Key, type_list<Lists...>>
{
    template <typename... Rest>
    static type_list<Key, Rest...> f(type_list<Key, Rest...> *);

    using derived = inherit<Lists...>;

    using type = decltype(f(static_cast<derived *>(nullptr)));
};

template <typename Key, typename Map>
using map_find = map_find_impl<Key, Map>::type;

// Second
template <typename List>
struct second_impl;

template <typename First, typename Second, typename... Rest>
struct second_impl<type_list<First, Second, Rest...>>
{
    using type = Second;
};

template <typename List>
using second = second_impl<List>::type;

// Count
template <typename List>
struct count_impl;

template <typename... Types>
struct count_impl<type_list<Types...>>
{
    static constexpr std::size_t value = sizeof...(Types);
};

template <typename List>
inline constexpr std::size_t count = count_impl<List>::value;

// Enumerate
template <typename List, typename Seq>
struct enumerate_impl;

template <typename... Types, std::size_t... Idxs>
struct enumerate_impl<type_list<Types...>, std::index_sequence<Idxs...>>
{
    using type = type_list<type_list<index<Idxs>, Types>...>;
};

template <typename List>
using enumerate = enumerate_impl<List, std::make_index_sequence<count<List>>>::type;

// Get nth (fast)
template <std::size_t N, typename List>
using get_nth_fast = second<map_find<index<N>, enumerate<List>>>;

} // namespace tl

// contains<T, List>
// flip<List>, List -- список пар
// index_of<T, List>
// flatten<List>
// merge_sort<List, Compare>

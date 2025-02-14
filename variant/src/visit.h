#pragma once
#include "get.h"
#include "help_structs.h"
#include "helpers.h"

#include <functional>
#include <type_traits>

namespace details {
template <typename T>
using result_tag = std::type_identity<T>;

template <typename T, size_t...>
struct table_t {
  static constexpr bool need_result = false;

  constexpr const T& operator()() const {
    return data;
  }

  T data;
};

template <typename... Args>
struct table_t<void (*)(Args...)> {
  using Type = void (*)(Args...);
  static constexpr bool need_result = false;

  constexpr const Type& operator()() const {
    return data;
  }

  Type data;
};

template <typename Res, typename... Args>
struct table_t<result_tag<Res> (*)(Args...)> {
  using Type = Res (*)(Args...);
  static constexpr bool need_result = true;

  constexpr const Type& operator()() const {
    return data;
  }

  Type data;
};

template <typename R, typename Visitor, typename... Variants, size_t first, size_t... rest>
struct table_t<R (*)(Visitor, Variants...), first, rest...> {
  template <typename... Args>
  constexpr decltype(auto) operator()(size_t first_index, Args... rest_indices) const {
    return data[first_index](rest_indices...);
  }

  table_t<R (*)(Visitor, Variants...), rest...> data[first];
};

template <typename, typename>
struct gen_table_impl;

template <typename R, typename Visitor, size_t... sizes, typename... Variants, size_t... Idx>
struct gen_table_impl<table_t<R (*)(Visitor, Variants...), sizes...>, std::index_sequence<Idx...>> {
  using type = table_t<R (*)(Visitor, Variants...), sizes...>;

  constexpr type operator()() {
    using next_var = typename std::remove_reference_t<nth_type_t<sizeof...(Idx), Variants...>>;
    return gen(std::make_index_sequence<variant_size_v<next_var>>());
  }

  template <size_t... indices>
  static constexpr type gen(std::index_sequence<indices...>) {
    type table{};
    (gen_helper<indices>(table.data[indices]), ...);
    return table;
  }

  template <size_t index, typename T>
  static constexpr void gen_helper(T& element) {
    element = gen_table_impl<std::remove_reference_t<decltype(element)>, std::index_sequence<Idx..., index>>{}();
  }
};

template <typename R, typename Visitor, typename... Variants, size_t... indices>
struct gen_table_impl<table_t<R (*)(Visitor, Variants...)>, std::index_sequence<indices...>> {
  using table_type = table_t<R (*)(Visitor, Variants...)>;

  static constexpr decltype(auto) invoke(Visitor&& visitor, Variants... vars) {
    if constexpr (table_type::need_result) {
      return std::invoke(std::forward<Visitor>(visitor), get<indices>(std::forward<Variants>(vars))...);
    } else {
      return static_cast<R>(std::invoke(std::forward<Visitor>(visitor), get<indices>(std::forward<Variants>(vars))...));
    }
  }

  constexpr auto operator()() const {
    return table_type{&invoke};
  }
};

template <typename Result_type, typename Visitor, typename... Variants>
struct gen_table {
  static constexpr table_t table = gen_table_impl<
      table_t<Result_type (*)(Visitor, Variants...), variant_size_v<std::remove_reference_t<Variants>>...>,
      std::index_sequence<>>{}();
};

template <typename Result_type, typename Visitor, typename... Variants>
constexpr decltype(auto) visit_impl(Visitor&& visitor, Variants&&... variants) {
  constexpr auto& table = gen_table<Result_type, Visitor&&, Variants&&...>::table;

  auto func_ptr = table(variants.index()...);
  return (*func_ptr)(std::forward<Visitor>(visitor), std::forward<Variants>(variants)...);
}

template <size_t Index, typename ResultType, typename Visitor, typename Variant>
constexpr ResultType invoke_with_index(Visitor&& visitor, Variant&& variant) {
  return std::invoke(
      std::forward<Visitor>(visitor),
      get<Index>(std::forward<Variant>(variant)),
      std::integral_constant<size_t, Index>{}
  );
}

template <typename ResultType, typename Visitor, typename Variant, typename FuncType, size_t... Indices>
constexpr void create_table_with_index(FuncType* table, std::index_sequence<Indices...>) {
  ((table[Indices] = &invoke_with_index<Indices, ResultType, Visitor, Variant>), ...);
}

template <typename ResultType, typename Visitor, typename Variant>
constexpr decltype(auto) visit_with_index(Visitor&& visitor, Variant&& variant) {
  constexpr size_t v_size = variant_size_v<std::remove_reference_t<Variant>>;

  using FuncType = ResultType (*)(Visitor&&, Variant&&);
  FuncType table[v_size];

  create_table_with_index<ResultType, Visitor, Variant>(table, std::make_index_sequence<v_size>{});

  return table[variant.index()](std::forward<Visitor>(visitor), std::forward<Variant>(variant));
}

} // namespace details

template <
    typename Visitor,
    typename... Variants,
    typename R = std::invoke_result_t<Visitor, variant_type<0, Variants>...>>
constexpr R visit(Visitor&& visitor, Variants&&... variants) {
  if ((as_variant(variants).valueless_by_exception() || ...)) {
    throw bad_variant_access("valueless variant in visit");
  }
  return details::visit_impl<details::result_tag<R>>(
      std::forward<Visitor>(visitor),
      as_variant(std::forward<Variants>(variants))...
  );
}

template <typename Res, typename Visitor, typename... Variants>
constexpr Res visit(Visitor&& visitor, Variants&&... variants) {
  if ((as_variant(variants).valueless_by_exception() || ...)) {
    throw bad_variant_access("valueless variant in visit");
  }
  return details::visit_impl<Res>(std::forward<Visitor>(visitor), as_variant(std::forward<Variants>(variants))...);
}

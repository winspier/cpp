#pragma once
#include "help_structs.h"
#include "helpers.h"

#include <compare>

template <typename... Types>
  requires (std::three_way_comparable<Types> && ...)
constexpr std::common_comparison_category_t<std::compare_three_way_result_t<Types>...>
operator<=>(const variant<Types...>& lhs, const variant<Types...>& rhs) {
  if (lhs.valueless_by_exception()) {
    return std::strong_ordering::less;
  }
  if (rhs.valueless_by_exception()) {
    return std::strong_ordering::greater;
  }
  if (lhs.index() != rhs.index()) {
    return lhs.index() <=> rhs.index();
  }

  std::common_comparison_category_t<std::compare_three_way_result_t<Types>...> res = std::strong_ordering::equal;

  rhs.visit_with_index([&res, &lhs](auto&& rhs_value, auto index) mutable {
    constexpr size_t rhs_index = decltype(index)::value;
    if constexpr (rhs_index != variant_npos) {
      if (lhs.index() == rhs_index) {
        auto& lhs_value = lhs.template get<rhs_index>();
        res = lhs_value <=> rhs_value;
      }
    }
  });
  return res;
}

template <typename... Types>
constexpr bool operator<(const variant<Types...>& lhs, const variant<Types...>& rhs) {
  if (rhs.valueless_by_exception()) {
    return false;
  }
  if (lhs.valueless_by_exception()) {
    return true;
  }
  if (lhs.index() < rhs.index()) {
    return true;
  }
  if (lhs.index() > rhs.index()) {
    return false;
  }

  bool res = true;
  rhs.visit_with_index([&res, &lhs](auto&& rhs_value, auto index) mutable {
    constexpr size_t rhs_index = decltype(index)::value;
    if constexpr (rhs_index != variant_npos) {
      if (lhs.index() == rhs_index) {
        auto& lhs_value = lhs.template get<rhs_index>();
        res = lhs_value < rhs_value;
      }
    }
  });
  return res;
}

template <typename... Types>
constexpr bool operator>(const variant<Types...>& lhs, const variant<Types...>& rhs) {
  if (lhs.valueless_by_exception()) {
    return false;
  }
  if (rhs.valueless_by_exception()) {
    return true;
  }
  if (lhs.index() > rhs.index()) {
    return true;
  }
  if (lhs.index() < rhs.index()) {
    return false;
  }

  bool res = true;
  rhs.visit_with_index([&res, &lhs](auto&& rhs_value, auto index) mutable {
    constexpr size_t rhs_index = decltype(index)::value;
    if constexpr (rhs_index != variant_npos) {
      if (lhs.index() == rhs_index) {
        auto& lhs_value = lhs.template get<rhs_index>();
        res = lhs_value > rhs_value;
      }
    }
  });
  return res;
}

template <typename... Types>
constexpr bool operator<=(const variant<Types...>& lhs, const variant<Types...>& rhs) {
  if (lhs.valueless_by_exception()) {
    return true;
  }
  if (rhs.valueless_by_exception()) {
    return false;
  }
  if (lhs.index() < rhs.index()) {
    return true;
  }
  if (lhs.index() > rhs.index()) {
    return false;
  }

  bool res = true;
  rhs.visit_with_index([&res, &lhs](auto&& rhs_value, auto index) mutable {
    constexpr size_t rhs_index = decltype(index)::value;
    if constexpr (rhs_index != variant_npos) {
      if (lhs.index() == rhs_index) {
        auto& lhs_value = lhs.template get<rhs_index>();
        res = lhs_value <= rhs_value;
      }
    }
  });
  return res;
}

template <typename... Types>
constexpr bool operator>=(const variant<Types...>& lhs, const variant<Types...>& rhs) {
  if (rhs.valueless_by_exception()) {
    return true;
  }
  if (lhs.valueless_by_exception()) {
    return false;
  }
  if (lhs.index() > rhs.index()) {
    return true;
  }
  if (lhs.index() < rhs.index()) {
    return false;
  }

  bool res = true;
  rhs.visit_with_index([&res, &lhs](auto&& rhs_value, auto index) mutable {
    constexpr size_t rhs_index = decltype(index)::value;
    if constexpr (rhs_index != variant_npos) {
      if (lhs.index() == rhs_index) {
        auto& lhs_value = lhs.template get<rhs_index>();
        res = lhs_value >= rhs_value;
      }
    }
  });
  return res;
}

template <typename... Types>
constexpr bool operator==(const variant<Types...>& lhs, const variant<Types...>& rhs) {
  if (lhs.index() != rhs.index()) {
    return false;
  }
  if (lhs.valueless_by_exception()) {
    return true;
  }
  bool res = true;
  rhs.visit_with_index([&res, &lhs](auto&& rhs_value, auto index) mutable {
    constexpr size_t rhs_index = decltype(index)::value;
    if constexpr (rhs_index != variant_npos) {
      if (lhs.index() == rhs_index) {
        auto& lhs_value = lhs.template get<rhs_index>();
        res = lhs_value == rhs_value;
      }
    }
  });
  return res;
}

template <typename... Types>
constexpr bool operator!=(const variant<Types...>& lhs, const variant<Types...>& rhs) {
  if (lhs.index() != rhs.index()) {
    return true;
  }
  if (lhs.valueless_by_exception()) {
    return false;
  }
  bool res = true;
  rhs.visit_with_index([&res, &lhs](auto&& rhs_value, auto index) mutable {
    constexpr size_t rhs_index = decltype(index)::value;
    if constexpr (rhs_index != variant_npos) {
      if (lhs.index() == rhs_index) {
        auto& lhs_value = lhs.template get<rhs_index>();
        res = lhs_value != rhs_value;
      }
    }
  });
  return res;
}

constexpr std::strong_ordering operator<=>(monostate, monostate) noexcept {
  return std::strong_ordering::equal;
}

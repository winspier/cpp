#pragma once
#include <type_traits>

namespace properties {

enum class property {
  DELETED,
  USER_DEFINED,
  TRIVIAL
};

template <typename T>
constexpr property get_dtor_property() {
  if constexpr (std::is_trivially_destructible_v<T>) {
    return property::TRIVIAL;
  }
  return property::USER_DEFINED;
}

template <typename T>
constexpr property get_copy_assign_property() {
  if constexpr (std::is_trivially_copy_assignable_v<T> && std::is_trivially_copy_constructible_v<T> &&
                std::is_trivially_destructible_v<T>) {
    return property::TRIVIAL;
  }
  if constexpr (!std::is_copy_constructible_v<T> || !std::is_copy_assignable_v<T>) {
    return property::DELETED;
  }
  return property::USER_DEFINED;
}

template <typename T>
constexpr property get_move_assign_property() {
  if constexpr (std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T> &&
                std::is_trivially_destructible_v<T>) {
    return property::TRIVIAL;
  }
  if constexpr (!std::is_move_constructible_v<T> || !std::is_move_assignable_v<T>) {
    return property::DELETED;
  }
  return property::USER_DEFINED;
}

template <typename T>
constexpr property get_move_ctor_property() {
  if constexpr (!std::is_move_constructible_v<T>) {
    return property::DELETED;
  }
  if constexpr (!std::is_trivially_move_constructible_v<T>) {
    return property::USER_DEFINED;
  }
  return property::TRIVIAL;
}

template <typename T>
constexpr property get_copy_ctor_property() {
  if constexpr (!std::is_copy_constructible_v<T>) {
    return property::DELETED;
  }
  if constexpr (!std::is_trivially_copy_constructible_v<T>) {
    return property::USER_DEFINED;
  }
  return property::TRIVIAL;
}
} // namespace properties

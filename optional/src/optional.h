#pragma once

#include "optional_impl.h"

#include <compare>
#include <exception>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

struct nullopt_t {
  explicit constexpr nullopt_t(int) {}
};

struct in_place_t {
  explicit in_place_t() = default;
};

inline constexpr nullopt_t nullopt{0};
inline constexpr in_place_t in_place{};

template <typename T>
class optional : optional_impl::optional_impl_base<T> {
public:
  using value_type = T;

  using optional_impl::optional_impl_base<T>::operator*;
  using optional_impl::optional_impl_base<T>::operator->;
  using optional_impl::optional_impl_base<T>::reset;
  using optional_impl::optional_impl_base<T>::has_value;
  using optional_impl::optional_impl_base<T>::value;

  constexpr optional() noexcept = default;

  constexpr optional(nullopt_t) noexcept
      : optional() {}

  constexpr optional(const optional&) = default;
  constexpr optional(optional&&) = default;

  constexpr optional& operator=(const optional&) = default;
  constexpr optional& operator=(optional&&) = default;

  template <
      typename U = T,
      std::enable_if_t<
          std::is_constructible_v<T, U&&> && !std::is_same_v<std::remove_cvref_t<U>, in_place_t> &&
              !std::is_same_v<std::remove_cvref_t<U>, optional>,
          bool> = true>
  explicit(!std::is_convertible_v<U&&, T>) constexpr optional(U&& value
  ) noexcept(std::is_nothrow_constructible_v<T, U>) {
    this->construct(std::forward<U>(value));
  }

  template <typename... Args>
  explicit constexpr optional(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args&&...>) {
    this->construct(std::forward<Args>(args)...);
  }

  constexpr optional& operator=(nullopt_t) noexcept {
    this->reset();
    return *this;
  }

  template <
      typename U = T,
      std::enable_if_t<
          std::is_constructible_v<T, U> && !std::is_same_v<std::remove_cvref_t<U>, optional> &&
              std::is_assignable_v<T&, U> && (!std::is_same_v<std::remove_cvref_t<U>, T> || !std::is_scalar_v<T>),
          bool> = true>
  constexpr optional& operator=(U&& value
  ) noexcept(std::is_nothrow_assignable_v<T&, U> && std::is_nothrow_constructible_v<T, U&&>) {
    if (this->has_value()) {
      this->assign(std::forward<U>(value));
    } else {
      this->construct(std::forward<U>(value));
    }
    return *this;
  }

  constexpr explicit operator bool() const noexcept {
    return this->has_value();
  }

  template <typename... Args>
  constexpr T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args&&...>) {
    this->reset();
    this->construct(std::forward<Args>(args)...);
    return **this;
  }
};

template <typename U, std::enable_if_t<std::is_swappable_v<U> && std::is_move_constructible_v<U>, bool> = true>
constexpr void swap(
    optional<U>& lhs,
    optional<U>& rhs
) noexcept(std::is_nothrow_swappable_v<U> && std::is_nothrow_move_constructible_v<U>) {
  if (rhs.has_value() && lhs.has_value()) {
    using std::swap;
    swap(*lhs, *rhs);
  } else if (rhs.has_value()) {
    lhs.emplace(std::move(*rhs));
    rhs.reset();
  } else if (lhs.has_value()) {
    swap(rhs, lhs);
  }
}

template <typename U, std::enable_if_t<!std::is_swappable_v<U> || !std::is_move_constructible_v<U>, bool> = true>
constexpr void swap(optional<U>& lhs, optional<U>& rhs) = delete;

template <typename T>
constexpr bool operator==(const optional<T>& lhs, const optional<T>& rhs) {
  return (bool(lhs) == bool(rhs) && (!bool(lhs) || *lhs == *rhs));
}

template <typename T>
constexpr bool operator!=(const optional<T>& lhs, const optional<T>& rhs) {
  return (bool(lhs) != bool(rhs) || (bool(lhs) && *lhs != *rhs));
}

template <typename T>
constexpr bool operator<(const optional<T>& lhs, const optional<T>& rhs) {
  return bool(rhs) && (!bool(lhs) || *lhs < *rhs);
}

template <typename T>
constexpr bool operator<=(const optional<T>& lhs, const optional<T>& rhs) {
  return !bool(lhs) || (bool(rhs) && *lhs <= *rhs);
}

template <typename T>
constexpr bool operator>(const optional<T>& lhs, const optional<T>& rhs) {
  return bool(lhs) && (!bool(rhs) || *lhs > *rhs);
}

template <typename T>
constexpr bool operator>=(const optional<T>& lhs, const optional<T>& rhs) {
  return !bool(rhs) || (bool(lhs) && *lhs >= *rhs);
}

template <class T>
constexpr std::compare_three_way_result_t<T> operator<=>(const optional<T>& lhs, const optional<T>& rhs) {
  return bool(lhs) && bool(rhs) ? *lhs <=> *rhs : bool(lhs) <=> bool(rhs);
}

template <typename T>
optional(T) -> optional<T>;

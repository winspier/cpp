#pragma once
#include "helpers.h"

#include <stdexcept>

namespace details {
template <bool trivial_dtor, typename... Types>
class variant_storage_impl {
public:
  variant_storage_impl() = default;

  template <size_t N, typename... Args>
  variant_storage_impl(in_place_index_t<N>, Args&&...) = delete;
};

template <bool trivial_dtor, typename First, typename... Rest>
class variant_storage_impl<trivial_dtor, First, Rest...> {
public:
  constexpr variant_storage_impl()
      : rest_() {}

  template <typename... Args>
  constexpr variant_storage_impl(in_place_index_t<0>, Args&&... args)
    requires std::is_constructible_v<First, Args&&...>
      : first_(std::forward<Args>(args)...) {}

  template <size_t N, typename... Args>
    requires (N <= sizeof...(Rest))
  constexpr variant_storage_impl(in_place_index_t<N>, Args&&... args)
      : rest_(in_place_index<N - 1>, std::forward<Args>(args)...) {}

  template <typename... Args>
  constexpr explicit variant_storage_impl(const size_t N, Args&&... args) {
    if (N == 0) {
      std::construct_at(std::addressof(first_), std::forward<Args>(args)...);
    } else {
      variant_storage_impl(N - 1, std::forward<Args>(args)...);
    }
  }

  template <size_t N, typename... Args>
  constexpr void emplace(in_place_index_t<N>, Args&&... args) {
    if constexpr (N == 0) {
      std::construct_at(std::addressof(first_), std::forward<Args>(args)...);
    } else {
      emplace(in_place_index<N - 1>, std::forward<Args>(args)...);
    }
  }

  template <size_t N>
  constexpr void reset(in_place_index_t<N>) {
    if constexpr (N == 0) {
      std::destroy_at(std::addressof(first_));
    } else {
      rest_.reset(N - 1);
    }
  }

  template <size_t N>
  constexpr auto& get() & {
    if constexpr (N == 0) {
      return first_;
    } else {
      return rest_.template get<N - 1>();
    }
  }

  template <size_t N>
  constexpr const auto& get() const& {
    if constexpr (N == 0) {
      return first_;
    } else {
      return rest_.template get<N - 1>();
    }
  }

  template <size_t N>
  constexpr auto&& get() && {
    if constexpr (N == 0) {
      return std::move(first_);
    } else {
      return std::move(rest_).template get<N - 1>();
    }
  }

  template <size_t N>
  constexpr const auto&& get() const&& {
    if constexpr (N == 0) {
      return std::move(first_);
    } else {
      return std::move(rest_).template get<N - 1>();
    }
  }

  constexpr ~variant_storage_impl() = default;

  constexpr ~variant_storage_impl()
    requires (!trivial_dtor)
  {}

private:
  union {
    First first_;
    variant_storage_impl<trivial_dtor, Rest...> rest_;
  };
};

template <typename... Types>
using variant_storage = variant_storage_impl<traits<Types...>::trivial_dtor, Types...>;
} // namespace details

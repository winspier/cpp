#pragma once
#include "comparisons.h"
#include "get.h"
#include "help_structs.h"
#include "helpers.h"
#include "traits.h"
#include "variant_storage.h"
#include "visit.h"

template <typename... Alternatives>
class variant {
  using First = details::first_type_t<Alternatives...>;
  using v_traits = details::traits<Alternatives...>;

public:
  constexpr variant(const variant&)
    requires v_traits::trivial_copy_ctor
  = default;
  constexpr variant(variant&&)
    requires v_traits::trivial_move_ctor
  = default;
  constexpr variant& operator=(const variant&)
    requires v_traits::trivial_copy_assign
  = default;
  constexpr variant& operator=(variant&&)
    requires v_traits::trivial_move_assign
  = default;
  ~variant()
    requires v_traits::trivial_dtor
  = default;

  constexpr variant() noexcept(std::is_nothrow_default_constructible_v<First>)
    requires std::is_default_constructible_v<First>
      : data_(in_place_index<0>)
      , index_(0) {}

  constexpr ~variant() noexcept {
    reset();
  }

  constexpr void reset() noexcept {
    if (index_ != variant_npos) {
      visit([](auto&& value) mutable { std::destroy_at(std::addressof(value)); });
      index_ = variant_npos;
    }
  }

  constexpr variant(const variant& other) noexcept(v_traits::nothrow_copy_ctor)
    requires v_traits::basic_copy_ctor
  {
    if (other.index() != variant_npos) {
      other.visit_with_index([this](auto&& value, auto index) mutable {
        std::construct_at(std::addressof(data_), in_place_index<decltype(index)::value>, value);
      });

      index_ = other.index_;
    }
  }

  constexpr variant(variant&& other) noexcept(v_traits::nothrow_move_ctor)
    requires v_traits::basic_move_ctor
  {
    if (other.index_ != variant_npos) {
      other.visit_with_index([this](auto&& value, auto index) mutable {
        std::construct_at(std::addressof(data_), in_place_index<decltype(index)::value>, std::move(value));
      });

      index_ = other.index();
    }
  }

  constexpr variant& operator=(const variant& other) noexcept(v_traits::nothrow_copy_assign)
    requires v_traits::basic_copy_assign
  {
    if (this != &other) {
      if (other.index() == variant_npos) {
        reset();
      } else {
        other.visit_with_index([this](auto&& value, auto index) mutable {
          constexpr size_t N = decltype(index)::value;
          if (N == this->index()) {
            this->get<N>() = value;
          } else {
            using T = details::nth_type_t<N, Alternatives...>;
            if constexpr (std::is_nothrow_copy_constructible_v<T> || !std::is_nothrow_move_constructible_v<T>) {
              this->template emplace<N>(value);
            } else {
              variant& tmp = details::variant_cast<Alternatives...>(*this);
              tmp = variant(in_place_index<N>, value);
            }
          }
        });
      }
    }
    return *this;
  }

  constexpr variant& operator=(variant&& other) noexcept(v_traits::nothrow_move_assign)
    requires v_traits::basic_move_assign
  {
    if (this != &other) {
      if (index_ == other.index_) {
        other.visit_with_index([this](auto&& value, auto index) mutable {
          constexpr size_t N = decltype(index)::value;
          this->get<N>() = std::move(value);
        });

      } else {
        reset();
        other.visit_with_index([this](auto&& value, auto index) mutable {
          constexpr size_t N = decltype(index)::value;
          this->template emplace<N>(std::move(value));
        });
      }
    }
    return *this;
  }

  template <typename T, typename... Args>
    requires details::IsExactlyOneOf<T, Alternatives...>
  constexpr void emplace(Args&&... args) {
    constexpr size_t index = details::IndexOf_v<T, Alternatives...>;
    emplace<index>(std::forward<Args>(args)...);
  }

  template <size_t N, typename... Args>
    requires (N < variant_size_v<variant>)
  constexpr void emplace(Args&&... args) {
    reset();
    std::construct_at(std::addressof(data_), in_place_index<N>, std::forward<Args>(args)...);
    index_ = N;
  }

  template <typename T>
    requires details::IsExactlyOneOf<T, Alternatives...>
  constexpr auto&& get() const {
    return get<T>(*this);
  }

  template <typename T>
    requires details::IsExactlyOneOf<T, Alternatives...>
  constexpr auto&& get() {
    return get<T>(*this);
  }

  template <size_t N>
  constexpr variant_alternative_t<N, variant>& get() & {
    return data_.template get<N>();
  }

  template <size_t N>
  constexpr const variant_alternative_t<N, variant>& get() const& {
    return data_.template get<N>();
  }

  template <size_t N>
  constexpr variant_alternative_t<N, variant>&& get() && {
    return std::move(data_).template get<N>();
  }

  template <size_t N>
  constexpr const variant_alternative_t<N, variant>&& get() const&& {
    return std::move(data_).template get<N>();
  }

  template <typename Visitor>
  constexpr void visit(Visitor&& visitor) & {
    return details::visit_impl<void>(std::forward<Visitor>(visitor), details::variant_cast<Alternatives...>(*this));
  }

  template <typename Visitor>
  constexpr void visit(Visitor&& visitor) const& {
    return details::visit_impl<void>(std::forward<Visitor>(visitor), details::variant_cast<Alternatives...>(*this));
  }

  template <typename Visitor>
  constexpr void visit(Visitor&& visitor) && {
    return details::visit_impl<void>(
        std::forward<Visitor>(visitor),
        details::variant_cast<Alternatives...>(std::move(*this))
    );
  }

  template <typename Visitor>
  constexpr void visit(Visitor&& visitor) const&& {
    return details::visit_impl<void>(
        std::forward<Visitor>(visitor),
        details::variant_cast<Alternatives...>(std::move(*this))
    );
  }

  template <typename Visitor>
  constexpr void visit_with_index(Visitor&& visitor) & {
    return details::visit_with_index<void>(
        std::forward<Visitor>(visitor),
        details::variant_cast<Alternatives...>(*this)
    );
  }

  template <typename Visitor>
  constexpr void visit_with_index(Visitor&& visitor) const& {
    return details::visit_with_index<void>(
        std::forward<Visitor>(visitor),
        details::variant_cast<Alternatives...>(*this)
    );
  }

  template <typename Visitor>
  constexpr void visit_with_index(Visitor&& visitor) && {
    return details::visit_with_index<void>(
        std::forward<Visitor>(visitor),
        variant_cast<Alternatives...>(std::move(*this))
    );
  }

  template <typename Visitor>
  constexpr void visit_with_index(Visitor&& visitor) const&& {
    return details::visit_with_index<void>(
        std::forward<Visitor>(visitor),
        variant_cast<Alternatives...>(std::move(*this))
    );
  }

  template <std::size_t N, class... Args>
  constexpr explicit variant(in_place_index_t<N>, Args&&... args)
    requires is_constructible<N, variant, Args&&...>
      : data_(in_place_index<N>, std::forward<Args>(args)...)
      , index_(N) {}

  template <class T, class... Args>
  constexpr explicit variant(in_place_type_t<T>, Args&&... args)
    requires is_constructible<details::IndexOf_v<T, Alternatives...>, variant, Args&&...>
      : variant(in_place_index<details::IndexOf_v<T, Alternatives...>>, std::forward<Args>(args)...) {}

  template <typename T, typename Nice = details::nice_ctor_type<T&&, Alternatives...>>
    requires (details::IsExactlyOneOf<Nice, Alternatives...> && std::is_constructible_v<Nice, T>)
  constexpr variant(T&& t) noexcept(std::is_nothrow_constructible_v<Nice, T>)
      : variant(in_place_index<details::nice_index_v<T, Alternatives...>>, std::forward<T>(t)) {}

  constexpr std::size_t index() const noexcept {
    return index_;
  }

  constexpr bool valueless_by_exception() const noexcept {
    return index_ == variant_npos;
  }

  template <typename T>
    requires (!std::is_same_v<std::remove_cvref_t<T>, variant> && details::is_nice_constructible<T &&, Alternatives...>)
  constexpr variant& operator=(T&& t) noexcept(details::is_nothrow_nice_ctor<T&&, Alternatives...>) {
    constexpr auto N = details::nice_index_v<T&&, Alternatives...>;

    if (index() == N) {
      get<N>() = std::forward<T>(t);
    } else {
      using Nice = details::nice_ctor_type<T&&, Alternatives...>;
      if constexpr (std::is_nothrow_constructible_v<Nice, T> || !std::is_nothrow_move_constructible_v<Nice>) {
        emplace<N>(std::forward<T>(t));
      } else {
        emplace<N>(Nice(std::forward<T>(t)));
      }
    }
    return *this;
  }

  constexpr void swap(variant& rhs
  ) noexcept((std::is_nothrow_swappable_v<Alternatives> && ...) && std::is_nothrow_move_constructible_v<variant>) {
    if (rhs.valueless_by_exception()) {
      if (!this->valueless_by_exception()) {
        rhs.swap(*this);
      }
      return;
    }

    rhs.visit_with_index([this, &rhs](auto&& value, auto rhs_index) mutable {
      constexpr size_t N = decltype(rhs_index)::value;
      if constexpr (N != variant_npos) {
        if (this->index() == N) {
          using std::swap;
          swap(this->get<N>(), value);
        } else {
          auto tmp(std::move(value));

          if (!this->valueless_by_exception()) {
            this->visit_with_index([&rhs](auto&& value, auto lhs_index) mutable {
              constexpr size_t I = decltype(lhs_index)::value;
              if constexpr (I != variant_npos) {
                rhs.template emplace<I>(std::move(value));
              }
            });
          } else {
            rhs.reset();
          }
          emplace<N>(std::move(tmp));
        }
      } else {
        reset();
      }
    });
  }

private:
  template <size_t N, typename V>
  friend constexpr decltype(auto) details::get_impl(V&&);

  details::variant_storage<Alternatives...> data_;
  size_t index_ = variant_npos;
};

template <typename... Types>
  requires ((std::is_move_constructible_v<Types> && ...) && (std::is_swappable_v<Types> && ...))
constexpr void swap(variant<Types...>& lhs, variant<Types...>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
  lhs.swap(rhs);
}

template <typename... Types>
  requires (!((std::is_move_constructible_v<Types> && ...) && (std::is_swappable_v<Types> && ...)))
constexpr void swap(variant<Types...>&, variant<Types...>&) = delete;

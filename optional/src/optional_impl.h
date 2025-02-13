#pragma once
#include "properties.h"

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace optional_impl {

template <typename T>
class optional_interface_base;

template <typename T, properties::property = properties::get_dtor_property<T>()>
class storage_base;

template <typename T, properties::property = properties::get_copy_ctor_property<T>()>
struct copy_constructor_base;

template <typename T, properties::property = properties::get_move_ctor_property<T>()>
struct move_constructor_base;

template <typename T, properties::property = properties::get_move_assign_property<T>()>
struct move_assignment_base;

template <typename T, properties::property = properties::get_copy_assign_property<T>()>
struct copy_assignment_base;

template <typename T>
using optional_impl_base = copy_assignment_base<T>;

template <typename T>
class storage_base<T, properties::property::TRIVIAL> {
protected:
  union {
    T obj;
  };

  bool is_active{false};

public:
  constexpr storage_base() {}

  constexpr ~storage_base() = default;

  constexpr void reset() noexcept {
    is_active = false;
  }

  constexpr storage_base(const storage_base&) = default;
  constexpr storage_base(storage_base&&) = default;
  constexpr storage_base& operator=(const storage_base&) = default;
  constexpr storage_base& operator=(storage_base&&) = default;
};

template <typename T>
class storage_base<T, properties::property::USER_DEFINED> {
protected:
  union {
    T obj;
  };

  bool is_active{false};

public:
  constexpr storage_base() {}

  constexpr ~storage_base() {
    reset();
  }

  constexpr void reset() {
    if (is_active) {
      std::destroy_at(std::addressof(obj));
      is_active = false;
    }
  }

  constexpr storage_base(const storage_base&) = default;
  constexpr storage_base(storage_base&&) = default;
  constexpr storage_base& operator=(const storage_base&) = default;
  constexpr storage_base& operator=(storage_base&&) = default;
};

template <typename T>
class optional_interface_base : storage_base<T> {
public:
  using storage_base<T>::storage_base;

  constexpr optional_interface_base() = default;
  constexpr optional_interface_base(optional_interface_base&&) = default;
  constexpr optional_interface_base(const optional_interface_base&) = default;
  constexpr optional_interface_base& operator=(const optional_interface_base&) = default;
  constexpr optional_interface_base& operator=(optional_interface_base&&) = default;

protected:
  template <typename... Args>
  constexpr void construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args&&...>) {
    std::construct_at(std::addressof(this->obj), std::forward<Args>(args)...);
    this->is_active = true;
  }

  template <typename U>
  constexpr void assign(U&& value
  ) noexcept(std::is_nothrow_assignable_v<T&, U> && std::is_nothrow_constructible_v<T, U&&>) {
    if (has_value()) {
      this->obj = std::forward<U>(value);
    } else {
      construct(std::forward<U>(value));
    }
  }

public:
  using storage_base<T>::reset;

  constexpr T& operator*() & noexcept {
    return this->obj;
  }

  constexpr const T& operator*() const& noexcept {
    return this->obj;
  }

  constexpr T* operator->() noexcept {
    return std::addressof(this->obj);
  }

  constexpr const T* operator->() const noexcept {
    return std::addressof(this->obj);
  }

  constexpr T&& operator*() && noexcept {
    return std::move(this->obj);
  }

  constexpr const T&& operator*() const&& noexcept {
    return std::move(this->obj);
  }

  constexpr T& value() {
    if (!has_value()) {
      throw std::runtime_error("Attempted to access value from an empty optional");
    }
    return this->obj;
  }

  constexpr bool has_value() const noexcept {
    return this->is_active;
  }
};

template <typename T>
struct copy_constructor_base<T, properties::property::USER_DEFINED> : optional_interface_base<T> {
  using optional_interface_base<T>::optional_interface_base;
  constexpr copy_constructor_base() = default;
  constexpr copy_constructor_base(copy_constructor_base&&) = default;
  constexpr copy_constructor_base& operator=(copy_constructor_base&&) = default;
  constexpr copy_constructor_base& operator=(const copy_constructor_base&) = default;

  constexpr copy_constructor_base(const copy_constructor_base& other
  ) noexcept(std::is_nothrow_copy_constructible_v<T>) {
    if (other.has_value()) {
      this->construct(*other);
    }
  }
};

template <typename T>
struct copy_constructor_base<T, properties::property::TRIVIAL> : optional_interface_base<T> {
  using optional_interface_base<T>::optional_interface_base;
};

template <typename T>
struct copy_constructor_base<T, properties::property::DELETED> : optional_interface_base<T> {
  using optional_interface_base<T>::optional_interface_base;
  constexpr copy_constructor_base() = default;
  constexpr copy_constructor_base(copy_constructor_base&&) = default;
  constexpr copy_constructor_base& operator=(copy_constructor_base&&) = default;
  constexpr copy_constructor_base& operator=(const copy_constructor_base&) = default;
  constexpr copy_constructor_base(const copy_constructor_base&) = delete;
};

template <typename T>
struct move_constructor_base<T, properties::property::USER_DEFINED> : copy_constructor_base<T> {
  using copy_constructor_base<T>::copy_constructor_base;
  constexpr move_constructor_base() = default;
  constexpr move_constructor_base(const move_constructor_base&) = default;
  constexpr move_constructor_base& operator=(const move_constructor_base&) = default;
  constexpr move_constructor_base& operator=(move_constructor_base&&) = default;

  constexpr move_constructor_base(move_constructor_base&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
    if (other.has_value()) {
      this->construct(std::move(*other));
    }
  }
};

template <typename T>
struct move_constructor_base<T, properties::property::TRIVIAL> : copy_constructor_base<T> {
  using copy_constructor_base<T>::copy_constructor_base;
};

template <typename T>
struct move_constructor_base<T, properties::property::DELETED> : copy_constructor_base<T> {
  using copy_constructor_base<T>::copy_constructor_base;
  constexpr move_constructor_base() = default;
  constexpr move_constructor_base(move_constructor_base&&) = delete;
  constexpr move_constructor_base(const move_constructor_base&) = default;
  constexpr move_constructor_base& operator=(const move_constructor_base&) = default;
  constexpr move_constructor_base& operator=(move_constructor_base&&) = default;
};

template <typename T>
struct move_assignment_base<T, properties::property::USER_DEFINED> : move_constructor_base<T> {
  using move_constructor_base<T>::move_constructor_base;
  constexpr move_assignment_base() = default;
  constexpr move_assignment_base(const move_assignment_base&) = default;
  constexpr move_assignment_base(move_assignment_base&&) = default;
  constexpr move_assignment_base& operator=(const move_assignment_base& other) = default;

  constexpr move_assignment_base& operator=(move_assignment_base&& other
  ) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) {
    if (other.has_value()) {
      this->assign(std::move(*other));
    } else {
      this->reset();
    }
    return *this;
  }
};

template <typename T>
struct move_assignment_base<T, properties::property::TRIVIAL> : move_constructor_base<T> {
  using move_constructor_base<T>::move_constructor_base;
};

template <typename T>
struct move_assignment_base<T, properties::property::DELETED> : move_constructor_base<T> {
  using move_constructor_base<T>::move_constructor_base;
  constexpr move_assignment_base() = default;
  constexpr move_assignment_base(const move_assignment_base&) = default;
  constexpr move_assignment_base(move_assignment_base&&) = default;
  constexpr move_assignment_base& operator=(const move_assignment_base& other) = default;
  constexpr move_assignment_base& operator=(move_assignment_base&& other) = delete;
};

template <typename T>
struct copy_assignment_base<T, properties::property::USER_DEFINED> : move_assignment_base<T> {
  using move_assignment_base<T>::move_assignment_base;
  constexpr copy_assignment_base() = default;
  constexpr copy_assignment_base(const copy_assignment_base&) = default;
  constexpr copy_assignment_base(copy_assignment_base&&) = default;
  constexpr copy_assignment_base& operator=(copy_assignment_base&&) = default;

  constexpr copy_assignment_base& operator=(const copy_assignment_base& other
  ) noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) {
    if (other.has_value()) {
      this->assign(*other);
    } else {
      this->reset();
    }
    return *this;
  }
};

template <typename T>
struct copy_assignment_base<T, properties::property::TRIVIAL> : move_assignment_base<T> {
  using move_assignment_base<T>::move_assignment_base;
};

template <typename T>
struct copy_assignment_base<T, properties::property::DELETED> : move_assignment_base<T> {
  using move_assignment_base<T>::move_assignment_base;
  constexpr copy_assignment_base() = default;
  constexpr copy_assignment_base(const copy_assignment_base&) = default;
  constexpr copy_assignment_base(copy_assignment_base&&) = default;
  constexpr copy_assignment_base& operator=(copy_assignment_base&&) = default;
  constexpr copy_assignment_base& operator=(const copy_assignment_base& other) = delete;
};

} // namespace optional_impl

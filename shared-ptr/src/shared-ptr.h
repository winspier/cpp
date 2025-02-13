#pragma once

#include "details.h"

#include <cstddef>
#include <memory>
#include <utility>

template <typename T, typename Y>
concept pointer_convertible = std::is_convertible_v<T*, Y*>;

template <typename T>
class weak_ptr;

template <typename T>
class shared_ptr {
  using control_block = details::control_block;

  template <typename U, typename Deleter>
  using control_block_ptr = details::control_block_ptr<U, Deleter>;

  template <typename U>
  using control_block_obj = details::control_block_obj<U>;

  template <typename U, typename... Args>
  friend shared_ptr<U> make_shared(Args&&... args);

  template <typename U>
  friend class shared_ptr;

  template <typename Y>
  friend class weak_ptr;

public:
  shared_ptr() noexcept
      : cb_(nullptr)
      , ptr_(nullptr) {}

  shared_ptr(std::nullptr_t) noexcept
      : shared_ptr() {}

private:
  void decrease_ref() const noexcept {
    if (cb_) {
      cb_->decrease_strong_ref();
    }
  }

  void increase_ref() const noexcept {
    if (cb_) {
      cb_->increase_strong_ref();
    }
  }

  explicit shared_ptr(control_block_obj<T>* cb) noexcept
      : cb_(cb)
      , ptr_(cb->get_data()) {}

  explicit shared_ptr(const weak_ptr<T>& other) noexcept
      : cb_(other.cb_)
      , ptr_(other.get()) {
    increase_ref();
  }

public:
  ~shared_ptr() noexcept {
    decrease_ref();
  }

  template <pointer_convertible<T> Y>
  explicit shared_ptr(Y* ptr)
      : shared_ptr(ptr, std::default_delete<Y>()) {}

  template <pointer_convertible<T> Y, typename Deleter>
  shared_ptr(Y* ptr, Deleter deleter) try
      : cb_(new control_block_ptr<Y, Deleter>(ptr, std::move(deleter)))
      , ptr_(ptr) {
  } catch (...) {
    deleter(ptr);
  }

  template <typename Y>
  shared_ptr(const shared_ptr<Y>& other, T* ptr) noexcept
      : cb_(other.cb_)
      , ptr_(ptr) {
    increase_ref();
  }

  template <typename Y>
  shared_ptr(shared_ptr<Y>&& other, T* ptr) noexcept
      : cb_(std::exchange(other.cb_, nullptr))
      , ptr_(ptr) {
    other.ptr_ = nullptr;
  }

  shared_ptr(const shared_ptr& other) noexcept
      : shared_ptr(other, other.get()) {}

  template <pointer_convertible<T> Y>
  shared_ptr(const shared_ptr<Y>& other) noexcept
      : shared_ptr(other, other.get()) {}

  template <pointer_convertible<T> Y>
  shared_ptr(shared_ptr<Y>&& other) noexcept
      : shared_ptr(std::move(other), other.get()) {}

  shared_ptr(shared_ptr&& other) noexcept
      : shared_ptr(std::move(other), other.get()) {}

  shared_ptr& operator=(const shared_ptr& other) noexcept {
    shared_ptr tmp(other);
    swap(tmp);
    return *this;
  }

  template <pointer_convertible<T> Y>
  shared_ptr& operator=(const shared_ptr<Y>& other) noexcept {
    shared_ptr tmp(other);
    swap(tmp);
    return *this;
  }

  shared_ptr& operator=(shared_ptr&& other) noexcept {
    shared_ptr tmp(std::move(other));
    swap(tmp);
    return *this;
  }

  template <pointer_convertible<T> Y>
  shared_ptr& operator=(shared_ptr<Y>&& other) noexcept {
    shared_ptr tmp(std::move(other));
    swap(tmp);
    return *this;
  }

  T* get() const noexcept {
    return ptr_;
  }

  operator bool() const noexcept {
    return ptr_ != nullptr;
  }

  T& operator*() const noexcept {
    return *ptr_;
  }

  T* operator->() const noexcept {
    return ptr_;
  }

  std::size_t use_count() const noexcept {
    return cb_ ? cb_->strong_ref_count() : 0;
  }

  void reset() noexcept {
    decrease_ref();
    cb_ = nullptr;
    ptr_ = nullptr;
  }

  template <pointer_convertible<T> Y>
  void reset(Y* new_ptr) {
    reset(new_ptr, std::default_delete<Y>());
  }

  template <typename Y, typename Deleter>
  void reset(Y* new_ptr, Deleter deleter) {
    *this = shared_ptr(new_ptr, std::move(deleter));
  }

  friend bool operator==(const shared_ptr& lhs, const shared_ptr& rhs) noexcept {
    return lhs.get() == rhs.get();
  }

  friend bool operator!=(const shared_ptr& lhs, const shared_ptr& rhs) noexcept {
    return !(lhs == rhs);
  }

  void swap(shared_ptr& other) noexcept {
    using std::swap;
    swap(cb_, other.cb_);
    swap(ptr_, other.ptr_);
  }

private:
  control_block* cb_;
  T* ptr_;
};

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
  return shared_ptr<T>(new details::control_block_obj<T>(std::forward<Args>(args)...));
}

template <typename T>
class weak_ptr {
  using control_block = details::control_block;

  template <typename U>
  friend class weak_ptr;

  template <typename U>
  friend class shared_ptr;

public:
  weak_ptr() noexcept
      : cb_(nullptr)
      , ptr_(nullptr) {}

  ~weak_ptr() noexcept {
    decrease_ref();
  }

private:
  void decrease_ref() const noexcept {
    if (cb_) {
      cb_->decrease_weak_ref();
    }
  }

  void increase_ref() const noexcept {
    if (cb_) {
      cb_->increase_weak_ref();
    }
  }

  weak_ptr(control_block* cb, T* ptr) noexcept
      : cb_(cb)
      , ptr_(ptr) {
    increase_ref();
  }

public:
  template <pointer_convertible<T> Y>
  weak_ptr(const shared_ptr<Y>& other) noexcept
      : weak_ptr(other.cb_, other.get()) {}

  weak_ptr(const weak_ptr& other) noexcept
      : weak_ptr(other.cb_, other.get()) {}

  T* get() const noexcept {
    return ptr_;
  }

  template <pointer_convertible<T> Y>
  weak_ptr(const weak_ptr<Y>& other) noexcept
      : weak_ptr(other.cb_, other.get()) {}

  weak_ptr(weak_ptr&& other) noexcept
      : cb_(std::exchange(other.cb_, nullptr))
      , ptr_(std::exchange(other.ptr_, nullptr)) {}

  template <pointer_convertible<T> Y>
  weak_ptr(weak_ptr<Y>&& other) noexcept
      : cb_(std::exchange(other.cb_, nullptr))
      , ptr_(std::exchange(other.ptr_, nullptr)) {}

  template <pointer_convertible<T> Y>
  weak_ptr& operator=(const shared_ptr<Y>& other) noexcept {
    weak_ptr tmp(other);
    swap(tmp);
    return *this;
  }

  weak_ptr& operator=(const weak_ptr& other) noexcept {
    weak_ptr tmp(other);
    swap(tmp);
    return *this;
  }

  template <pointer_convertible<T> Y>
  weak_ptr& operator=(const weak_ptr<Y>& other) noexcept {
    weak_ptr tmp(other);
    swap(tmp);
    return *this;
  }

  weak_ptr& operator=(weak_ptr&& other) noexcept {
    weak_ptr tmp(std::move(other));
    swap(tmp);
    return *this;
  }

  template <pointer_convertible<T> Y>
  weak_ptr& operator=(weak_ptr<Y>&& other) noexcept {
    weak_ptr tmp(std::move(other));
    swap(tmp);
    return *this;
  }

  bool expired() const noexcept {
    return !cb_ || cb_->strong_ref_count() == 0;
  }

  shared_ptr<T> lock() const noexcept {
    return expired() ? shared_ptr<T>() : shared_ptr<T>(*this);
  }

  void reset() noexcept {
    decrease_ref();
    cb_ = nullptr;
    ptr_ = nullptr;
  }

  void swap(weak_ptr other) noexcept {
    using std::swap;
    swap(cb_, other.cb_);
    swap(ptr_, other.ptr_);
  }

private:
  control_block* cb_;
  T* ptr_;
};

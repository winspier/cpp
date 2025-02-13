#pragma once
#include <memory>

template <typename T>
class shared_ptr;

namespace details {
class control_block {
protected:
  virtual ~control_block() noexcept = default;
  virtual void die() noexcept = 0;

public:
  control_block() noexcept = default;

  void decrease_strong_ref() noexcept {
    if (--strong_ref_count_ == 0) {
      die();
      if (weak_ref_count_ == 0) {
        delete this;
      }
    }
  }

  void decrease_weak_ref() noexcept {
    if (--weak_ref_count_ == 0 && strong_ref_count_ == 0) {
      delete this;
    }
  }

  void increase_strong_ref() noexcept {
    ++strong_ref_count_;
  }

  void increase_weak_ref() noexcept {
    ++weak_ref_count_;
  }

  std::size_t strong_ref_count() const noexcept {
    return strong_ref_count_;
  }

  std::size_t weak_ref_count() const noexcept {
    return weak_ref_count_;
  }

private:
  std::size_t strong_ref_count_{1};
  std::size_t weak_ref_count_{0};
};

template <typename T, typename Deleter>
class control_block_ptr final : public control_block {
public:
  explicit control_block_ptr(T* ptr, Deleter deleter)
      : ptr_(ptr)
      , deleter_(std::move(deleter)) {}

protected:
  ~control_block_ptr() noexcept override = default;

  void die() noexcept override {
    deleter_(ptr_);
  }

private:
  T* ptr_;
  [[no_unique_address]] Deleter deleter_;
};

template <typename T>
class control_block_obj final : public control_block {
public:
  template <typename... Args>
  explicit control_block_obj(Args&&... args)
      : obj_(std::forward<Args>(args)...) {}

  T* get_data() noexcept {
    return &obj_;
  }

protected:
  ~control_block_obj() noexcept override {}

  void die() noexcept override {
    obj_.~T();
  }

private:
  union {
    T obj_;
  };
};

} // namespace details

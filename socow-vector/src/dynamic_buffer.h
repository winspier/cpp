#pragma once

#include <cstddef>
#include <memory>

template <typename T, std::size_t SMALL_SIZE>
class socow_vector;

template <typename T>
class dynamic_buffer {
  template <typename, std::size_t>
  friend class socow_vector;

private:
  size_t capacity;
  size_t ref_count;
  T data[0];

private:
  dynamic_buffer(size_t size)
      : capacity(size)
      , ref_count(1) {}

  static dynamic_buffer* create(size_t size) {
    void* buf = operator new(sizeof(dynamic_buffer) + sizeof(T) * size);
    dynamic_buffer* res = new (buf) dynamic_buffer(size);
    return res;
  }

  static dynamic_buffer* create(const dynamic_buffer* other, size_t size) {
    return create(other, size, other->capacity);
  }

  static dynamic_buffer* create(const dynamic_buffer* other, size_t size, size_t capacity) {
    auto buf = create(capacity);
    try {
      std::uninitialized_copy_n(other->data, size, buf->data);
    } catch (...) {
      operator delete(buf);
      throw;
    }
    return buf;
  }

  bool unique() {
    return ref_count == 1;
  }

  void add_copy() {
    ++ref_count;
  }

  void remove_copy(size_t size) {
    if (ref_count > 0) {
      --ref_count;
    }
    if (ref_count == 0) {
      destroy(this, size);
    }
  }

  static void destroy(dynamic_buffer* buf, size_t size) {
    std::destroy_n(buf->data, size);
    operator delete(buf);
  }
};

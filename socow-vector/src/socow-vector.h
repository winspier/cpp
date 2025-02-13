#pragma once

#include "dynamic_buffer.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>

template <typename T, std::size_t SMALL_SIZE>
class socow_vector {
public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using buffer_type = dynamic_buffer<T>;

public:
  socow_vector() noexcept
      : _size(0)
      , _use_static_data(true) {}

  socow_vector(size_t capacity)
      : _size(0)
      , _use_static_data(capacity <= SMALL_SIZE) {
    if (!_use_static_data) {
      _dynamic_data = buffer_type::create(capacity);
    }
  }

  socow_vector(const socow_vector& other)
      : socow_vector(other, other.capacity()) {}

private:
  socow_vector(const socow_vector& other, size_t capacity) {
    make_copy(other, capacity);
  }

public:
  socow_vector(socow_vector&& other) noexcept {
    make_move(std::move(other));
  }

private:
  void make_copy(const socow_vector& other, size_t capacity) {
    if (capacity <= SMALL_SIZE) {
      std::uninitialized_copy_n(other.begin(), other.size(), _static_data);
      _use_static_data = true;
    } else if (other.capacity() == capacity) {
      _dynamic_data = other._dynamic_data;
      _dynamic_data->add_copy();
      _use_static_data = false;
    } else {
      _dynamic_data = buffer_type::create(other._dynamic_data, other.size(), capacity);
      _use_static_data = false;
    }
    _size = other.size();
  }

  void make_move(socow_vector&& other) noexcept {
    _use_static_data = other._use_static_data;
    _size = other.size();
    if (_use_static_data) {
      std::uninitialized_move_n(other._static_data, other.size(), _static_data);
      other.destroy();
    } else {
      _dynamic_data = other._dynamic_data;
      other._dynamic_data = nullptr;
      other._size = 0;
      other._use_static_data = true;
    }
  }

public:
  socow_vector& operator=(const socow_vector& other) {
    if (&other != this) {
      socow_vector tmp(other);
      destroy();
      swap(tmp);
    }
    return *this;
  }

  socow_vector& operator=(socow_vector&& other) noexcept {
    if (&other != this) {
      destroy();
      make_move(std::move(other));
    }
    return *this;
  }

  ~socow_vector() {
    destroy();
  }

private:
  void unshare() {
    unshare(size(), size());
  }

  void unshare(size_t reserve_size, size_t new_capacity) {
    if (is_shared_data()) {
      if (reserve_size <= SMALL_SIZE) {
        auto tmp = _dynamic_data;
        try {
          std::uninitialized_copy_n(tmp->data, size(), _static_data);
        } catch (...) {
          _dynamic_data = tmp;
          throw;
        }
        tmp->remove_copy(size());
        _use_static_data = true;
      } else {
        buffer_type* new_buffer = buffer_type::create(_dynamic_data, size(), new_capacity);
        _dynamic_data->remove_copy(size());
        _dynamic_data = new_buffer;
        _use_static_data = false;
      }
    }
  }

public:
  reference operator[](std::size_t index) {
    return data()[index];
  }

  const_reference operator[](std::size_t index) const noexcept {
    return data()[index];
  }

  pointer data() {
    unshare();
    return _use_static_data ? _static_data : _dynamic_data->data;
  }

  const_pointer data() const noexcept {
    return _use_static_data ? _static_data : _dynamic_data->data;
  }

  std::size_t size() const noexcept {
    return _size;
  }

  reference front() {
    return (*this)[0];
  }

  const_reference front() const noexcept {
    return (*this)[0];
  }

  reference back() {
    return (*this)[size() - 1];
  }

  const_reference back() const noexcept {
    return (*this)[size() - 1];
  }

  void pop_back() {
    if (is_unshared_data()) {
      --_size;
      std::destroy_at(end());
      return;
    }
    erase(clean_begin() + size() - 1);
  }

  bool empty() const noexcept {
    return size() == 0;
  }

  std::size_t capacity() const noexcept {
    return _use_static_data ? SMALL_SIZE : _dynamic_data->capacity;
  }

private:
  bool is_unshared_data() const noexcept {
    return _use_static_data || _dynamic_data->unique();
  }

  bool is_shared_data() const noexcept {
    return !is_unshared_data();
  }

public:
  void reserve(std::size_t reserve_size) {
    if (capacity() < reserve_size || (size() < reserve_size && is_shared_data())) {
      resize(reserve_size);
    }
    if (size() < reserve_size && is_shared_data()) {
      unshare(reserve_size, capacity());
    }
  }

  void resize(size_t new_capacity) {
    if (is_unshared_data()) {
      if (new_capacity <= SMALL_SIZE) {
        if (!_use_static_data) {
          auto buf = _dynamic_data;
          _use_static_data = true;
          std::uninitialized_move_n(buf->data, size(), clean_begin());
          buf->remove_copy(size());
        }
        return;
      }
      buffer_type* tmp = buffer_type::create(new_capacity);
      std::uninitialized_move_n(clean_begin(), size(), tmp->data);
      if (_use_static_data) {
        std::destroy_n(begin(), size());
      } else {
        _dynamic_data->remove_copy(size());
      }
      _dynamic_data = tmp;
      _use_static_data = false;
    } else {
      socow_vector tmp(*this, new_capacity);
      swap(tmp);
    }
  }

  void shrink_to_fit() {
    if (_use_static_data || size() == capacity()) {
      return;
    }
    resize(size());
  }

  void clear() noexcept {
    if (is_unshared_data()) {
      std::destroy_n(clean_begin(), size());
    } else {
      _dynamic_data->remove_copy(size());
      _use_static_data = true;
    }
    _size = 0;
  }

private:
  void destroy() noexcept {
    if (_use_static_data) {
      std::destroy_n(clean_begin(), size());
    } else if (_dynamic_data) {
      _dynamic_data->remove_copy(size());
      _dynamic_data = nullptr;
    }
    _use_static_data = true;
    _size = 0;
  }

public:
  void swap(socow_vector& other) noexcept {
    if (&other == this) {
      return;
    }
    using std::swap;
    if (_use_static_data && other._use_static_data && size() <= other.size()) {
      for (size_t i = 0; i < std::min(size(), other.size()); i++) {
        swap(_static_data[i], other._static_data[i]);
      }
      std::uninitialized_move_n(other._static_data + size(), other.size() - size(), _static_data + size());
      std::destroy_n(other._static_data + size(), other.size() - size());
    } else if (!other._use_static_data) {
      if (_use_static_data) {
        auto tmp = other._dynamic_data;
        std::uninitialized_move_n(_static_data, size(), other._static_data);
        std::destroy_n(_static_data, size());
        _dynamic_data = tmp;
      } else {
        swap(_dynamic_data, other._dynamic_data);
      }
    } else {
      other.swap(*this);
      return;
    }
    swap(_size, other._size);
    swap(_use_static_data, other._use_static_data);
  }

  iterator begin() {
    return data();
  }

private:
  iterator clean_begin() noexcept {
    return _use_static_data ? _static_data : _dynamic_data->data;
  }

  iterator clean_end() noexcept {
    return clean_begin() + size();
  }

public:
  iterator end() {
    return begin() + size();
  }

  const_iterator begin() const noexcept {
    return data();
  }

  const_iterator end() const noexcept {
    return begin() + size();
  }

  void push_back(const value_type& value) {
    insert(clean_end(), value);
  }

  void push_back(value_type&& value) {
    insert(clean_end(), std::move(value));
  }

  iterator insert(const_iterator pos, const value_type& value) {
    return insert_impl(pos, value);
  }

  iterator insert(const_iterator pos, value_type&& value) {
    return insert_impl(pos, std::move(value));
  }

private:
  template <class R>
  iterator insert_impl(const_iterator pos, R&& value) {
    std::size_t ind = pos - clean_begin();

    if (size() < capacity() && is_unshared_data()) {
      new (clean_end()) value_type(std::forward<R>(value));
      ++_size;

      iterator pos_it = clean_begin() + ind;
      std::rotate(pos_it, clean_end() - 1, clean_end());
      return pos_it;
    }

    socow_vector tmp(2 * size() + 1);
    if (is_unshared_data()) {
      new (tmp.clean_begin() + ind) value_type(std::forward<R>(value));
      for (auto it = clean_begin(); it != clean_begin() + ind; ++it) {
        tmp.push_back(std::move(*it));
      }
      tmp._size++;
      for (auto it = clean_begin() + ind; it != clean_end(); ++it) {
        tmp.push_back(std::move(*it));
      }
    } else {
      for (auto it = clean_begin(); it != clean_begin() + ind; ++it) {
        tmp.push_back(*it);
      }
      tmp.push_back(std::forward<R>(value));
      for (auto it = clean_begin() + ind; it != clean_end(); ++it) {
        tmp.push_back(*it);
      }
    }
    swap(tmp);
    return clean_begin() + ind;
  }

public:
  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }

  iterator erase(const_iterator first, const_iterator last) {
    std::size_t ind_first = first - clean_begin();
    std::size_t ind_last = last - clean_begin();
    if (is_unshared_data()) {
      std::move(clean_begin() + ind_last, clean_begin() + size(), clean_begin() + ind_first);
      _size = size() - ind_last + ind_first;
      std::destroy_n(clean_begin() + size(), ind_last - ind_first);
    } else {
      socow_vector tmp(size() - ind_last + ind_first);
      for (size_t i = 0; i < ind_first; i++) {
        tmp.push_back(std::as_const(*this)[i]);
      }
      for (size_t i = ind_last; i < size(); i++) {
        tmp.push_back(std::as_const(*this)[i]);
      }
      swap(tmp);
    }
    return clean_begin() + ind_first;
  }

private:
  union {
    buffer_type* _dynamic_data;
    value_type _static_data[SMALL_SIZE];
  };

  size_t _size;
  bool _use_static_data;
};

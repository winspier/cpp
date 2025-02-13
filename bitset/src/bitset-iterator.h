#pragma once

#include "bitset.h"
#include "bitset-reference.h"

#include <cstddef>
#include <iterator>

template <typename T>
class bitset_iterator {
public:
  using value_type = bool;
  using difference_type = std::ptrdiff_t;
  using reference = bitset_reference<T>;
  using pointer = void;
  using word_pointer = T*;
  using iterator_category = std::random_access_iterator_tag;

  bitset_iterator() = default;

  bitset_iterator(const bitset_iterator &other)
      : _current(other._current), _index(other._index) {}

private:
  friend class bitset;
  static constexpr size_t word_size = sizeof(T) * 8;

  bitset_iterator(word_pointer current, size_t index)
      : _current(current)
      , _index(index) {}

public:
  operator bitset_iterator<const T>() const {
    return {_current, _index};
  }

  reference operator*() const {
    return {_current, word_size - _index - 1};
  }

  bitset_iterator& operator++() {
    if (_index + 1 < word_size) {
      ++_index;
    } else {
      ++_current;
      _index = 0;
    }
    return *this;
  }

  bitset_iterator operator++(int) {
    bitset_iterator tmp = *this;
    ++*this;
    return tmp;
  }

  bitset_iterator& operator--() {
    if (_index > 0) {
      --_index;
    } else {
      --_current;
      _index = word_size - 1;
    }
    return *this;
  }

  bitset_iterator operator--(int) {
    bitset_iterator tmp = *this;
    --*this;
    return tmp;
  }

  bitset_iterator& operator+=(difference_type n) {
    _current += n / static_cast<difference_type>(word_size);
    difference_type new_index = static_cast<difference_type>(_index) + n % static_cast<difference_type>(word_size);
    if (new_index < 0) {
      new_index += static_cast<difference_type>(word_size);
      ++_current;
    } else if (new_index >= word_size) {
      new_index -= static_cast<difference_type>(word_size);
      --_current;
    }
    _index = static_cast<size_t>(new_index);
    return *this;
  }

  bitset_iterator& operator-=(difference_type n) {
    return *this + (-n);
  }

  friend bitset_iterator operator+(const bitset_iterator& a, difference_type n) {
    return bitset_iterator(a) += n;
  }

  friend bitset_iterator operator+(difference_type n, const bitset_iterator& a) {
    return bitset_iterator(a) += n;
  }

  friend bitset_iterator operator-(const bitset_iterator& a, difference_type n) {
    return base_col_iterator(a) -= n;
  }

  reference operator[](difference_type n) const {
    return *(*this + n);
  }

  friend difference_type operator-(const bitset_iterator& lhs, const bitset_iterator& rhs) {
    return (lhs._current - rhs._current) * static_cast<difference_type>(word_size) +
           static_cast<difference_type>(lhs._index) - static_cast<difference_type>(rhs._index);
  }

  friend auto operator<=>(const bitset_iterator& lhs, const bitset_iterator& rhs) {
    if (lhs._current == rhs._current) return lhs._index <=> rhs._index;
    return lhs._current <=> rhs._current;
  }

  friend bool operator==(const bitset_iterator& lhs, const bitset_iterator& rhs) = default;
  friend bool operator!=(const bitset_iterator& lhs, const bitset_iterator& rhs) = default;

private:
  word_pointer _current;
  size_t _index;
};
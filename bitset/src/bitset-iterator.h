#pragma once

#include "bitset-reference.h"
#include "bts.h"

#include <bit>
#include <cstddef>
#include <iterator>
#include <sstream>
#include <string>

template <typename T>
class bitset_iterator {
public:
  using value_type = bool;
  using word_type = T;
  using difference_type = std::ptrdiff_t;
  using reference = bitset_reference<word_type>;
  using pointer = void;
  using word_pointer = word_type*;
  using iterator_category = std::random_access_iterator_tag;

  bitset_iterator() = default;

private:
  friend class bitset;

  template <typename R>
  friend class bitset_iterator;

  template <typename R>
  friend class bitset_view;

  bitset_iterator(word_pointer current, size_t index)
      : _current(current)
      , _index(index) {}

  word_type get_high() const {
    return (*_current) << _index;
  }

  word_type get_low() const {
    return (*_current) >> (bts::word_size - _index);
  }

  word_type get_word(size_t size = bts::word_size) const {
    const word_type word_mask = (bts::word_ones << (bts::word_size - size));
    if (bts::word_size - _index >= size) {
      return get_high() & word_mask;
    }
    bitset_iterator next_word = {_current + 1, _index};
    return (get_high() | next_word.get_low()) & word_mask;
  }

public:
  operator bitset_iterator<const T>() const {
    return {_current, _index};
  }

  reference operator*() const {
    return {_current, bts::word_size - _index - 1};
  }

  bitset_iterator& operator++() {
    if (_index + 1 < bts::word_size) {
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
      _index = bts::word_size - 1;
    }
    return *this;
  }

  bitset_iterator operator--(int) {
    bitset_iterator tmp = *this;
    --*this;
    return tmp;
  }

  bitset_iterator& operator+=(difference_type n) {
    _current += n / static_cast<difference_type>(bts::word_size);
    difference_type new_index = static_cast<difference_type>(_index) + n % static_cast<difference_type>(bts::word_size);
    if (new_index < 0) {
      new_index += static_cast<difference_type>(bts::word_size);
      --_current;
    } else if (new_index >= bts::word_size) {
      new_index -= static_cast<difference_type>(bts::word_size);
      ++_current;
    }
    _index = static_cast<size_t>(new_index);
    return *this;
  }

  bitset_iterator& operator-=(difference_type n) {
    *this += (-n);
    return *this;
  }

  reference operator[](difference_type n) const {
    return *(*this + n);
  }

  friend difference_type operator-(const bitset_iterator& lhs, const bitset_iterator& rhs) {
    return (lhs._current - rhs._current) * static_cast<difference_type>(bts::word_size) +
           static_cast<difference_type>(lhs._index) - static_cast<difference_type>(rhs._index);
  }

  friend std::strong_ordering operator<=>(const bitset_iterator& lhs, const bitset_iterator& rhs) {
    if (lhs._current == rhs._current) {
      return lhs._index <=> rhs._index;
    }
    return lhs._current <=> rhs._current;
  }

  friend bool operator==(const bitset_iterator& lhs, const bitset_iterator& rhs) = default;
  friend bool operator!=(const bitset_iterator& lhs, const bitset_iterator& rhs) = default;

  friend bitset_iterator operator+(const bitset_iterator& a, difference_type n) {
    bitset_iterator res = a;
    res += n;
    return res;
  }

  friend bitset_iterator operator+(difference_type n, const bitset_iterator& a) {
    bitset_iterator res = a;
    res += n;
    return res;
  }

  friend bitset_iterator operator-(const bitset_iterator& a, difference_type n) {
    bitset_iterator res = a;
    res -= n;
    return res;
  }

  void swap(bitset_iterator& other) noexcept {
    std::swap(_current, other._current);
    std::swap(_index, other._index);
  }

  friend void swap(bitset_iterator& lhs, bitset_iterator& rhs) noexcept {
    lhs.swap(rhs);
  }

private:
  word_pointer _current;
  size_t _index;
};

#pragma once

#include <bit>
#include <concepts>
#include <cstddef>
#include <sstream>
#include <string>

template <typename T>
class bitset_reference {
public:
  using word_type = T;
  using pointer = T*;

  operator bitset_reference<const T>() const {
    return {_word, _index};
  }

  operator bool() const {
    return (((*_word) >> _index) & word_type(1)) > word_type(0);
  }

  bitset_reference operator=(bool value) const
    requires (!std::is_const_v<T>)
  {
    if (*this != value) {
      flip();
    }
    return *this;
  }

  bitset_reference operator&=(bool value) const
    requires (!std::is_const_v<T>)
  {
    return *this = (bool(*this) & value);
  }

  bitset_reference operator|=(bool value) const
    requires (!std::is_const_v<T>)
  {
    return *this = (bool(*this) | value);
  }

  bitset_reference operator^=(bool value) const
    requires (!std::is_const_v<T>)
  {
    return *this = (bool(*this) ^ value);
  }

  bitset_reference flip() const
    requires (!std::is_const_v<T>)
  {
    (*_word) ^= word_type(1) << _index;
    return *this;
  }

private:
  template <typename R>
  friend class bitset_iterator;

  template <typename R>
  friend class bitset_reference;

  bitset_reference(pointer word, size_t index)
      : _word(word)
      , _index(index) {}

private:
  pointer _word;
  size_t _index;
};

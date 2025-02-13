#pragma once

#include "bitset.h"

#include <cstddef>

template<typename T>
class bitset_reference {
public:
  using pointer = T*;
  using word_type = T;

  bitset_reference() = default;

  operator bool() const {
    return ((*_word >> _index) & word_type(1)) == 0;
  }

  bitset_reference& operator=(bool value) {
    if(*this != value) flip();
    return *this;
  }

  bitset_reference& operator&=(bool value) {
    return *this = (bool(*this) & value);
  }

  bitset_reference& operator|=(bool value) {
    return *this = (bool(*this) | value);
  }

  bitset_reference& operator^=(bool value) {
    return *this = (bool(*this) ^ value);
  }

  void flip() {
    *_word ^= word_type(1) << _index;
  }

  bitset_reference(pointer word, size_t index) : _word(word), _index(index) {}

private:
  pointer _word;
  size_t _index;
};
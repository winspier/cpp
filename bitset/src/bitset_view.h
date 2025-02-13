#pragma once

#include "bitset.h"

template<typename T>
class bitset_view {
public:
  using value_type = bool;
  using word_type = T;
  using iterator = bitset_iterator<word_type>;
  using reference = bitset_reference<word_type>;
  using const_reference = bitset_reference<const word_type>;

  bitset_view() = default;

  bitset_view(const iterator& begin, const iterator& end);

  bitset_view(const bitset_view& other);

  bitset_view& operator=(const bitset_view &other) &;

//  bitset_view(const bitset& src);

  operator bitset_view<const T>() const;

  size_t size() const;

  iterator begin() const;

  iterator end() const;

  size_t count() const;

  reference operator[](std::size_t index) const;

  bitset_view& operator&=(const bitset_view& other) &;
  //  bitset_view& operator|=(const bitset_view& other) &;
  //  bitset_view& operator^=(const bitset_view& other) &;
  //  bitset_view& operator<<=(std::size_t count) &;
  //  bitset_view& operator>>=(std::size_t count) &;
  //  void flip() &;
  //
  //  bitset_view& set() &;
  //  bitset_view& reset() &;
  //
  //  bool all() const;
  //  bool any() const;
  //  std::size_t count() const;

private:
  iterator _begin;
  iterator _end;
};

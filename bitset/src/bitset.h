#pragma once

#include "bitset-iterator.h"
#include "bitset-reference.h"
#include "bitset-view.h"
#include "bts.h"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>

class bitset {
public:
  using word_type = bts::word_type;
  using word_pointer = word_type*;
  using value_type = bool;
  using reference = bitset_reference<word_type>;
  using const_reference = bitset_reference<const word_type>;
  using iterator = bitset_iterator<word_type>;
  using const_iterator = bitset_iterator<const word_type>;
  using view = bitset_view<word_type>;
  using const_view = bitset_view<const word_type>;

public:
  static constexpr std::size_t npos = -1;

  bitset();

  bitset(std::size_t size, bool value);
  bitset(const bitset& other);
  explicit bitset(std::string_view str);
  explicit bitset(const const_view& other);
  bitset(const_iterator first, const_iterator last);

  bitset& operator=(const bitset& other) &;
  bitset& operator=(std::string_view str) &;
  bitset& operator=(const const_view& other) &;

  ~bitset();

  void swap(bitset& other) noexcept;

  std::size_t size() const;
  bool empty() const;

  reference operator[](std::size_t index);
  const_reference operator[](std::size_t index) const;

  iterator begin();
  const_iterator begin() const;

  iterator end();
  const_iterator end() const;

  bitset& operator&=(const const_view& other) &;
  bitset& operator|=(const const_view& other) &;
  bitset& operator^=(const const_view& other) &;
  bitset& operator<<=(std::size_t count) &;
  bitset& operator>>=(std::size_t count) &;
  void flip() &;

  bitset& set() &;
  bitset& reset() &;

  bool all() const;
  bool any() const;
  std::size_t count() const;

  operator const_view() const;
  operator view();

  view subview(std::size_t offset = 0, std::size_t count = npos);
  const_view subview(std::size_t offset = 0, std::size_t count = npos) const;

  std::string to_string() const;

  void clear();

  friend bool operator==(const bitset::const_view& lhs, const bitset::const_view& rhs);
  friend bool operator!=(const bitset::const_view& lhs, const bitset::const_view& rhs);

private:
  explicit bitset(size_t);
  size_t size_in_words() const;

  template <bool changeable = true, typename word_operation_type>
  static void apply_operation(const const_view& lhs, const const_view& rhs, word_operation_type word_operation);

private:
  size_t _size;
  word_pointer _data;
};

bitset operator&(const bitset::const_view& lhs, const bitset::const_view& rhs);
bitset operator|(const bitset::const_view& lhs, const bitset::const_view& rhs);
bitset operator^(const bitset::const_view& lhs, const bitset::const_view& rhs);
bitset operator~(const bitset::const_view& bs);
bitset operator<<(const bitset::const_view& bs, std::size_t count);
bitset operator>>(const bitset::const_view& bs, std::size_t count);

bool operator==(const bitset::const_view& lhs, const bitset::const_view& rhs);
bool operator!=(const bitset::const_view& lhs, const bitset::const_view& rhs);

bool operator==(const bitset& left, const bitset& right);
bool operator!=(const bitset& left, const bitset& right);

void swap(bitset& lhs, bitset& rhs) noexcept;
std::string to_string(const bitset& bs);
std::ostream& operator<<(std::ostream& out, const bitset& bs);

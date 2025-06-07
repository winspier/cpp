#pragma once

#include "bitset-iterator.h"
#include "bitset-reference.h"
#include "bts.h"

#include <algorithm>
#include <bit>
#include <concepts>
#include <functional>
#include <numeric>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>

template <typename T>
class bitset_view {
public:
  using value_type = bool;
  using word_type = T;
  using iterator = bitset_iterator<word_type>;
  using const_iterator = bitset_iterator<const word_type>;
  using reference = bitset_reference<word_type>;
  using const_reference = bitset_reference<const word_type>;
  using view = bitset_view<word_type>;
  using const_view = bitset_view<const word_type>;

public:
  static constexpr std::size_t npos = -1;

  bitset_view() = default;

  bitset_view(const iterator& begin, const iterator& end)
      : _begin(begin)
      , _end(end) {}

  void swap(view& other) noexcept {
    std::swap(_begin, other._begin);
    std::swap(_end, other._end);
  }

  friend void swap(view& lhs, view& rhs) noexcept {
    lhs.swap(rhs);
  }

  operator const_view() const {
    return {_begin, _end};
  }

  std::size_t size() const {
    return end() - begin();
  }

  iterator begin() const {
    return _begin;
  }

  iterator end() const {
    return _end;
  }

  reference operator[](std::size_t index) const {
    return begin()[index];
  }

  view operator&=(const const_view& other) const
    requires (!std::is_const_v<T>)
  {
    return apply_operation(other, [](word_type& a, word_type b, size_t op_size) {
      bts::set_subword(a, a & b, 0, op_size);
      return true;
    });
  }

  view operator|=(const const_view& other) const
    requires (!std::is_const_v<T>)
  {
    return apply_operation(other, [](word_type& a, word_type b, size_t op_size) {
      bts::set_subword(a, a | b, 0, op_size);
      return true;
    });
  }

  view operator^=(const const_view& other) const
    requires (!std::is_const_v<T>)
  {
    return apply_operation(other, [](word_type& a, word_type b, size_t op_size) {
      bts::set_subword(a, a ^ b, 0, op_size);
      return true;
    });
  }

  view flip() const
    requires (!std::is_const_v<T>)
  {
    return apply_operation(*this, [](word_type& a, word_type, size_t op_size) {
      bts::set_subword(a, ~a, 0, op_size);
      return true;
    });
  }

  view set() const
    requires (!std::is_const_v<T>)
  {
    return apply_operation(*this, [](word_type& a, word_type, size_t op_size) {
      bts::set_subword(a, bts::word_ones, 0, op_size);
      return true;
    });
  }

  view reset() const
    requires (!std::is_const_v<T>)
  {
    return apply_operation(*this, [](word_type& a, word_type, size_t op_size) {
      bts::set_subword(a, bts::word_zeros, 0, op_size);
      return true;
    });
  }

  std::size_t count() const {
    std::size_t res = 0;
    apply_operation<false>(*this, [&res](const word_type&, word_type b, size_t op_size) {
      res += std::popcount((bts::get_subword(b, 0, op_size)));
      return true;
    });
    return res;
  }

  bool all() const {
    bool res = true;
    apply_operation<false>(*this, [&res](const word_type&, word_type b, size_t op_size) {
      res &= bts::get_subword(b, 0, op_size) == bts::get_subword(bts::word_ones, 0, op_size);
      return res;
    });
    return res;
  }

  bool any() const {
    bool res = false;
    apply_operation<false>(*this, [&res](const word_type&, word_type b, size_t op_size) {
      res |= bts::get_subword(b, 0, op_size) > 0;
      return !res;
    });
    return res;
  }

  bool empty() const {
    return size() == 0;
  }

  friend std::ostream& operator<<(std::ostream& out, const bitset_view& bs) {
    for (auto it : bs) {
      out << it;
    }
    return out;
  }

  std::string to_string() const {
    std::string res;
    res.reserve(size());
    for (auto it : *this) {
      res += it ? '1' : '0';
    }
    return res;
  }

  view subview(std::size_t offset = 0, std::size_t count = npos) const {
    if (size() <= offset) {
      return {end(), end()};
    }
    return {begin() + offset, begin() + (count >= size() - offset ? size() : offset + count)};
  }

private:
  friend class bitset;

  template <bool changeable = true, typename word_operation_type>
  view apply_operation(const const_view& other, const word_operation_type word_operation) const {
    if (!other.empty()) {
      auto it = begin();
      auto r_it = other.begin();
      for (std::size_t i = 0; i * bts::word_size < other.size(); it += bts::word_size, r_it += bts::word_size, ++i) {
        size_t operation_size = std::min(other.size() - i * bts::word_size, bts::word_size);
        word_type tmp = it.get_word(operation_size);
        if (!word_operation(tmp, r_it.get_word(operation_size), operation_size)) {
          return *this;
        }
        if constexpr (changeable) {
          update_bitset_word(it, tmp, operation_size);
        }
      }
    }
    return *this;
  }

  void update_bitset_word(iterator it, word_type value, size_t op_size) const
    requires (!std::is_const_v<T>)
  {
    if (bts::word_size - it._index < op_size) {
      *(it._current) = (it.get_low() << (bts::word_size - it._index)) | (value >> it._index);

      size_t next_index = (op_size + it._index - bts::word_size);
      bitset_iterator next_word = {it._current + 1, next_index};
      *(next_word._current) = (next_word.get_high() >> next_index) | (value << (bts::word_size - it._index));
    } else {
      word_type mask = ~bts::get_subword(bts::word_ones, it._index, op_size);
      *(it._current) = (value >> it._index) | (*(it._current) & mask);
    }
  }

private:
  iterator _begin;
  iterator _end;
};

template <typename T>
std::string to_string(const bitset_view<T>& bs) {
  return bs.to_string();
}

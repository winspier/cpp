#pragma once

#include <cstdint>

namespace bts {
using word_type = uint64_t;

constexpr size_t word_size = sizeof(word_type) * 8;
constexpr word_type word_ones = ~word_type(0);
constexpr word_type word_zeros = word_type(0);

inline word_type get_subword(word_type w, size_t index, size_t size) {
  return w & ((word_ones << (word_size - size)) >> index);
}

inline void set_subword(word_type& lhs, word_type rhs, size_t index, size_t size) {
  word_type mask = get_subword(word_ones, index, size);
  lhs = (lhs & (~mask)) | (rhs & mask);
}

inline word_type stow(std::basic_string_view<char> src) {
  word_type res = 0;
  size_t len = std::min(word_size, src.size());
  for (size_t i = 0; i < len; i++) {
    res = (res << 1) | word_type(src[i] == '1' ? 1 : 0);
  }
  res <<= word_size - len;
  return res;
}

} // namespace bts

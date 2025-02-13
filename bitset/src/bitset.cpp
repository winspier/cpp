#include "bitset.h"

#include <algorithm>

bitset::bitset(std::size_t size, bool value)
    : _size(size), _data(new word_type[size_in_words()]) {
  if (value) flip();
}

void bitset::flip() & {
  for (size_t i = 0; i < size_in_words(); i++) {
    _data[i] = ~_data[i];
  }
}

std::size_t bitset::size() const { return _size; }

size_t bitset::size_in_words() const {
  return (size() + word_size - 1) / word_size;
}

bitset::~bitset() {
  delete[] _data;
}

void bitset::swap(bitset& other) noexcept {
  std::swap(_data, other._data);
  std::swap(_size, other._size);
}

bool bitset::empty() const {
  return size() == 0;
}

bitset::reference bitset::operator[](std::size_t index) {
  return begin()[index];
}

bitset::iterator bitset::begin() {
  return {_data, size_t(0)};
}

bitset::const_reference bitset::operator[](std::size_t index) const {
  return begin()[index];
}

bitset::const_iterator bitset::begin() const {
  return {_data, size_t(0)};
}

bitset::iterator bitset::end() {
  return begin() + size();
}

bitset::const_iterator bitset::end() const {
  return begin() + size();
}

bitset::bitset(const bitset& other)
    : _size(other.size()), _data(other.empty() ? nullptr : new word_type[other.size_in_words()]) {
    std::copy_n(other._data, other.size_in_words(), _data);
}

bitset& bitset::operator=(const bitset& other) & {
  if (&other == this) {
    return *this;
  }
  bitset tmp(other);
  swap(tmp);
  return *this;
}

bitset::operator const_view() const {
  return bitset::const_view(begin(), end());
}

bitset::operator view() {
  return bitset::view(begin(), end());
}

bitset::view bitset::subview(std::size_t offset, std::size_t count) {
  return bitset::view(begin() + offset, begin() + offset + count);
}

bitset::const_view bitset::subview(std::size_t offset, std::size_t count) const {
  return bitset::const_view(begin() + offset, begin() + offset + count);
}

bitset::bitset(std::string_view str)
    : _size(str.size()), _data(new word_type[size_in_words()]) {
  for (size_t i = 0; i < str.size(); i++) {
    (*this)[i] = (str[i] == '1');
  }
}

bitset::bitset(const bitset::const_view& other)
    : _size(other.size()), _data(new word_type[size_in_words()]) {
  for (size_t i = 0; i < other.size(); ++i) {
    (*this)[i] = other[i];
  }
}

bitset::bitset(const bitset::const_iterator &first, const bitset::const_iterator& last)
    : bitset::bitset(bitset::const_view(first, last)) {}

bitset& bitset::operator=(std::string_view str) & {
  return *this = bitset(str);
}

bitset& bitset::operator=(const bitset::const_view& other) & {
  return *this = bitset(other);
}

bitset& bitset::operator&=(const bitset::const_view& other) & {
  for (size_t i = 0; i < other.size(); i++) {
    (*this)[i] &= other[i];
  }
  return *this;
}

bitset& bitset::operator|=(const bitset::const_view& other) & {
  for (size_t i = 0; i < other.size(); i++) {
    (*this)[i] |= other[i];
  }
  return *this;
}

bitset& bitset::operator^=(const bitset::const_view& other) & {
  for (size_t i = 0; i < other.size(); i++) {
    (*this)[i] ^= other[i];
  }
  return *this;
}

bitset& bitset::operator<<=(std::size_t count) & {
  for (size_t i = size() - 1; i > 0; i--) {
    (*this)[i] = i > count && (*this)[i - count];
  }
  return *this;
}

bitset& bitset::operator>>=(std::size_t count) & {
  for (size_t i = 0; i < size(); i++) {
    (*this)[i] = i + count < size() && (*this)[i + count];
  }
  return *this;
}

bitset& bitset::set() & {
  for (size_t i = 0; i < size_in_words(); i++) {
    _data[i] = word_ones;
  }
  return *this;
}

bitset& bitset::reset() & {
  for (size_t i = 0; i < size_in_words(); i++) {
    _data[i] = word_zeros;
  }
  return *this;
}

bool bitset::all() const {
  for (size_t i = 0; i < size(); i++) {
    if(!(*this)[i]) return false;
  }
  return true;
}

bool bitset::any() const {
  for (size_t i = 0; i < size(); i++) {
    if((*this)[i]) return true;
  }
  return false;
}

std::size_t bitset::count() const {
  size_t cnt = 0;
  for (size_t i = 0; i < size(); i++) {
    if((*this)[i]) cnt++;
  }
  return cnt;
}

bitset operator&(const bitset::const_view& lhs, const bitset::const_view& rhs) {
  bitset tmp(lhs);
  return tmp &= rhs;
}

bitset operator|(const bitset::const_view& lhs, const bitset::const_view& rhs) {
  bitset tmp(lhs);
  return tmp |= rhs;
}

bitset operator^(const bitset::const_view& lhs, const bitset::const_view& rhs) {
  bitset tmp(lhs);
  return tmp ^= rhs;
}

void swap(bitset& lhs, bitset& rhs) noexcept {
  lhs.swap(rhs);
}

bool operator==(const bitset& left, const bitset& right) {
  if (left.size() != right.size()) return false;
  for (
      auto l_it = left.begin(),
           r_it = right.begin();
      l_it != left.end(); ++l_it, ++r_it) {
    if (*l_it != *r_it) return false;
  }
  return true;
}

bool operator!=(const bitset& left, const bitset& right) {
  return !(left == right);
}

std::string to_string(const bitset& bs) {
  std::string str;
  for (auto b : bs) {
    str += b ? '1' : '0';
  }
  return str;
}

std::ostream& operator<<(std::ostream& out, const bitset& bs) {
  out << to_string(bs);
  return out;
}

bitset operator~(const bitset::const_view& bs) {
  bitset tmp(bs);
  tmp.flip();
  return tmp;
}

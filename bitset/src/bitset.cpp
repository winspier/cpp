#include "bitset.h"

bitset::bitset(std::size_t size, bool value)
    : bitset(size) {
  word_type word_value = (value ? bts::word_ones : bts::word_zeros);
  for (size_t i = 0; i < size_in_words(); i++) {
    new (_data + i) word_type(word_value);
  }
}

void bitset::flip() & {
  view(*this).flip();
}

std::size_t bitset::size() const {
  return _size;
}

size_t bitset::size_in_words() const {
  return (size() + bts::word_size - 1) / bts::word_size;
}

bitset::~bitset() {
  clear();
  operator delete(_data);
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
    : bitset(other.size()) {
  std::uninitialized_copy_n(other._data, other.size_in_words(), _data);
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
  return {begin(), end()};
}

bitset::operator view() {
  return {begin(), end()};
}

bitset::view bitset::subview(std::size_t offset, std::size_t count) {
  return view(*this).subview(offset, count);
}

bitset::const_view bitset::subview(std::size_t offset, std::size_t count) const {
  return const_view(*this).subview(offset, count);
}

bitset::bitset(std::string_view str)
    : bitset(str.size()) {
  for (size_t i = 0; i < size_in_words(); i++) {
    new (_data + i) word_type(bts::stow(str.substr(bts::word_size * i, bts::word_size)));
  }
}

bitset::bitset(const bitset::const_view& other)
    : bitset(other.size()) {
  if (!other.empty()) {
    auto it = other.begin();
    for (size_t i = 0; i * bts::word_size < other.size(); it += bts::word_size, ++i) {
      new (_data + i) word_type(it.get_word(std::min(other.size() - i * bts::word_size, bts::word_size)));
    }
  }
}

bitset::bitset(bitset::const_iterator first, bitset::const_iterator last)
    : bitset::bitset(bitset::const_view(first, last)) {}

bitset& bitset::operator=(std::string_view str) & {
  bitset tmp(str);
  swap(tmp);
  return *this;
}

bitset& bitset::operator=(const bitset::const_view& other) & {
  if (other.begin() == begin() && other.end() == end()) {
    return *this;
  }
  bitset tmp(other);
  swap(tmp);
  return *this;
}

bitset& bitset::operator&=(const bitset::const_view& other) & {
  view(*this) &= other;
  return *this;
}

bitset& bitset::operator|=(const bitset::const_view& other) & {
  view(*this) |= other;
  return *this;
}

bitset& bitset::operator^=(const bitset::const_view& other) & {
  view(*this) ^= other;
  return *this;
}

bitset& bitset::operator<<=(std::size_t count) & {
  if (count == 0) {
    return *this;
  }
  bitset tmp(size() + count, false);
  std::copy_n(_data, size_in_words(), tmp._data);
  swap(tmp);
  return *this;
}

bitset& bitset::operator>>=(std::size_t count) & {
  if (count >= size()) {
    clear();
    return *this;
  }
  *this = subview(0, size() - count);
  return *this;
}

bitset& bitset::set() & {
  view(*this).set();
  return *this;
}

bitset& bitset::reset() & {
  view(*this).reset();
  return *this;
}

bool bitset::all() const {
  return const_view(*this).all();
}

bool bitset::any() const {
  return const_view(*this).any();
}

std::size_t bitset::count() const {
  return const_view(*this).count();
}

bitset::bitset()
    : _size(0)
    , _data(nullptr) {}

std::string bitset::to_string() const {
  return const_view(*this).to_string();
}

bitset operator&(const bitset::const_view& lhs, const bitset::const_view& rhs) {
  bitset tmp(lhs);
  tmp &= rhs;
  return tmp;
}

bitset operator|(const bitset::const_view& lhs, const bitset::const_view& rhs) {
  bitset tmp(lhs);
  tmp |= rhs;
  return tmp;
}

bitset operator^(const bitset::const_view& lhs, const bitset::const_view& rhs) {
  bitset tmp(lhs);
  tmp ^= rhs;
  return tmp;
}

void swap(bitset& lhs, bitset& rhs) noexcept {
  lhs.swap(rhs);
}

std::string to_string(const bitset& bs) {
  return bs.to_string();
}

std::ostream& operator<<(std::ostream& out, const bitset& bs) {
  out << bitset::const_view(bs);
  return out;
}

bitset operator~(const bitset::const_view& bs) {
  bitset tmp(bs);
  tmp.flip();
  return tmp;
}

bool operator==(const bitset::const_view& lhs, const bitset::const_view& rhs) {
  if (lhs.size() != rhs.size()) {
    return false;
  }
  bool res = true;
  bitset::apply_operation<false>(lhs, rhs, [&res](bts::word_type a, bts::word_type b, size_t op_size) {
    res &= bts::get_subword(a, 0, op_size) == bts::get_subword(b, 0, op_size);
    return res;
  });
  return res;
}

bool operator!=(const bitset::const_view& lhs, const bitset::const_view& rhs) {
  return !(lhs == rhs);
}

bitset::bitset(size_t size)
    : _size(size)
    , _data(size == 0 ? nullptr : static_cast<word_pointer>(operator new((size_in_words()) * sizeof(word_type)))) {}

void bitset::clear() {
  std::destroy_n(_data, size_in_words());
  _size = 0;
}

template <bool changeable, typename word_operation_type>
void bitset::apply_operation(
    const bitset::const_view& lhs,
    const bitset::const_view& rhs,
    word_operation_type word_operation
) {
  lhs.apply_operation<changeable>(rhs, word_operation);
}

bool operator==(const bitset& left, const bitset& right) {
  return left.subview() == right.subview();
}

bool operator!=(const bitset& left, const bitset& right) {
  return !(left == right);
}

bitset operator<<(const bitset::const_view& bs, std::size_t count) {
  bitset tmp(bs);
  tmp <<= count;
  return tmp;
}

bitset operator>>(const bitset::const_view& bs, std::size_t count) {
  bitset tmp(bs);
  tmp >>= count;
  return tmp;
}

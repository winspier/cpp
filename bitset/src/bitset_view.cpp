#include "bitset_view.h"

template <typename T>
bitset_view<T>::operator bitset_view<const T>() const {
  return {_begin, _end};
}

template <typename T>
bitset_view<T>::iterator bitset_view<T>::end() const {
  return _end;
}

template <typename T>
size_t bitset_view<T>::count() const {
  return 0;
}

template <typename T>
bitset_view<T>::iterator bitset_view<T>::begin() const {
  return _begin;
}

template <typename T>
size_t bitset_view<T>::size() const {
  return end() - begin();
}

template <typename T>
bitset_view<T>::reference bitset_view<T>::operator[](std::size_t index) const {
  return begin()[index];
}

template <typename T>
bitset_view<T>::bitset_view(const bitset_view& other)
    : _begin(other._begin), _end(other._end) {}

template <typename T>
bitset_view<T>::bitset_view(const bitset_view::iterator& begin, const bitset_view::iterator& end)
    : _begin(begin), _end(end) {}

template <typename T>
bitset_view<T>& bitset_view<T>::operator=(const bitset_view& other) & {
  _begin = other._begin;
  _end = other._end;
  return *this;
}

template <typename T>
bitset_view<T>& bitset_view<T>::operator&=(const bitset_view& other) & {
  auto tmp = bitset(begin(), end());
  return tmp &= bitset(other);
}

template<typename T>
bool operator==(const bitset_view<T> &lhs, const bitset_view<T> &rhs) {
  return bitset(lhs) == bitset(rhs);
}

template<typename T>
bool operator!=(const bitset_view<T> &lhs, const bitset_view<T> &rhs) {
  return !(lhs == rhs);
}
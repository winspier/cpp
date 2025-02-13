#pragma once

#include "bimap_details.h"
#include "bst.h"

#include <cstddef>
#include <iterator>
#include <ranges>
#include <utility>

template <
    typename Left,
    typename Right,
    typename CompareLeft = std::less<Left>,
    typename CompareRight = std::less<Right>>
class bimap {
  using left_tag = details::left_tag;
  using right_tag = details::right_tag;

  using bst_element_left = details::bst_element_left;
  using bst_element_right = details::bst_element_right;

  using node_t = details::node_with_value<Left, Right>;
  using sent_t = details::node_base;

  using iterator = details::bimap_iterator<Left, Right, CompareLeft, CompareRight>;

public:
  using left_t = Left;
  using right_t = Right;

  using left_iterator = typename iterator::left_iterator;
  using right_iterator = typename iterator::right_iterator;

  bimap(CompareLeft compare_left = CompareLeft(), CompareRight compare_right = CompareRight())
      : left_(static_cast<bst_element_left*>(&sent_), std::move(compare_left))
      , right_(static_cast<bst_element_right*>(&sent_), std::move(compare_right)) {}

  bimap(const bimap& other)
      : bimap(other.left_.get_comparator(), other.right_.get_comparator()) {
    for (auto it = other.begin_left(); it != other.end_left(); ++it) {
      insert(*it, *it.flip());
    }
  }

  bimap(bimap&& other) noexcept
      : sent_(std::move(other.sent_))
      , left_(static_cast<bst_element_left*>(&sent_), std::move(other.left_))
      , right_(static_cast<bst_element_right*>(&sent_), std::move(other.right_))
      , size_(std::exchange(other.size_, 0)) {}

  bimap& operator=(const bimap& other) {
    if (this != &other) {
      bimap copy(other);
      swap(*this, copy);
    }
    return *this;
  }

  bimap& operator=(bimap&& other) noexcept {
    if (this != &other) {
      clear();
      swap(*this, other);
    }
    return *this;
  }

  void clear() noexcept {
    erase_left(begin_left(), end_left());
  }

  ~bimap() noexcept {
    clear();
  }

  friend void swap(bimap& lhs, bimap& rhs) noexcept {
    using std::swap;
    swap(lhs.sent_, rhs.sent_);
    swap(lhs.left_, rhs.left_);
    swap(lhs.right_, rhs.right_);
    swap(lhs.size_, rhs.size_);
  }

  left_iterator insert(const left_t& left, const right_t& right) {
    return insert_impl(left, right);
  }

  left_iterator insert(const left_t& left, right_t&& right) {
    return insert_impl(left, std::move(right));
  }

  left_iterator insert(left_t&& left, const right_t& right) {
    return insert_impl(std::move(left), right);
  }

  left_iterator insert(left_t&& left, right_t&& right) {
    return insert_impl(std::move(left), std::move(right));
  }

private:
  template <typename L, typename R>
  left_iterator insert_impl(L&& left, R&& right) {
    auto left_pos = left_.find_position(left);
    auto right_pos = right_.find_position(right);

    left_iterator left_it = {left_pos.get_iterator()};
    right_iterator right_it = {right_pos.get_iterator()};
    if (left_pos.inserted()) {
      return left_it.flip() == right_it ? left_it : end_left();
    }
    if (right_pos.inserted()) {
      return end_left();
    }
    auto* new_node = new node_t(std::forward<L>(left), std::forward<R>(right));
    right_.insert(right_pos, *new_node);
    size_++;
    return left_.insert(left_pos, *new_node);
  }

public:
  left_iterator erase_left(left_iterator it) noexcept {
    if (it == end_left()) {
      return it;
    }

    right_.erase(it.flip());
    left_iterator res = left_.erase(it);
    delete it.get_node();
    size_--;
    return res;
  }

  right_iterator erase_right(right_iterator it) noexcept {
    if (it == end_right()) {
      return it;
    }
    left_.erase(it.flip());
    right_iterator res = right_.erase(it);
    delete it.get_node();
    size_--;
    return res;
  }

  bool erase_left(const left_t& left) {
    auto it = find_left(left);
    if (it == end_left()) {
      return false;
    }
    erase_left(it);
    return true;
  }

  bool erase_right(const right_t& right) {
    auto it = find_right(right);
    if (it == end_right()) {
      return false;
    }
    erase_right(it);
    return true;
  }

  left_iterator erase_left(left_iterator first, left_iterator last) noexcept {
    left_iterator it = first;
    for (; it != last && it != end_left(); it = erase_left(it)) {}
    return it;
  }

  right_iterator erase_right(right_iterator first, right_iterator last) noexcept {
    right_iterator it = first;
    for (; it != last && it != end_right(); it = erase_right(it)) {}
    return it;
  }

  left_iterator find_left(const left_t& left) const {
    return {left_.find(left)};
  }

  right_iterator find_right(const right_t& value) const {
    return {right_.find(value)};
  }

  const right_t& at_left(const left_t& value) const {
    left_iterator it = left_.find(value);
    if (it != end_left()) {
      return *it.flip();
    }
    throw std::out_of_range("bimap::at_left");
  }

  const left_t& at_right(const right_t& key) const {
    right_iterator it = right_.find(key);
    if (it != end_right()) {
      return *it.flip();
    }
    throw std::out_of_range("bimap::at_right");
  }

  const right_t& at_left_or_default(const left_t& left_key)
    requires (std::is_default_constructible_v<right_t>)
  {
    left_iterator it = left_.find(left_key);
    if (it != end_left()) {
      return *it.flip();
    }
    auto right_pos = right_.find_position(right_t{});
    if (!right_pos.inserted()) {
      return *insert(left_key, right_t{}).flip();
    }

    right_iterator right_it = {right_pos.get_iterator()};
    auto left_it = right_it.flip();
    auto left_pos = left_.find_position(left_key);

    node_t* new_node = new node_t(left_key, right_t());

    left_.insert(left_pos, *new_node);

    left_.erase(left_it);
    right_.insert(right_pos, *new_node);
    delete right_it.get_node();
    return new_node->get_right();
  }

  const left_t& at_right_or_default(const right_t& right_key)
    requires (std::is_default_constructible_v<left_t>)
  {
    right_iterator it = right_.find(right_key);
    if (it != end_right()) {
      return *it.flip();
    }
    auto left_pos = left_.find_position(left_t{});
    if (!left_pos.inserted()) {
      return *insert(left_t{}, right_key);
    }

    left_iterator left_it = {left_pos.get_iterator()};
    auto right_it = left_it.flip();
    auto right_pos = right_.find_position(right_key);

    node_t* new_node = new node_t(left_t(), right_key);

    right_.insert(right_pos, *new_node);

    right_.erase(right_it);
    left_.insert(left_pos, *new_node);
    delete left_it.get_node();
    return new_node->get_left();
  }

  left_iterator lower_bound_left(const left_t& left) const {
    return left_.lower_bound(left);
  }

  left_iterator upper_bound_left(const left_t& left) const {
    return left_.upper_bound(left);
  }

  right_iterator lower_bound_right(const right_t& right) const {
    return right_.lower_bound(right);
  }

  right_iterator upper_bound_right(const right_t& right) const {
    return right_.upper_bound(right);
  }

  left_iterator begin_left() const noexcept {
    return {left_.begin()};
  }

  left_iterator end_left() const noexcept {
    return {left_.end()};
  }

  right_iterator begin_right() const noexcept {
    return {right_.begin()};
  }

  right_iterator end_right() const noexcept {
    return {right_.end()};
  }

  bool empty() const noexcept {
    return size() == 0;
  }

  std::size_t size() const noexcept {
    return size_;
  }

private:
  bool equal_left(const left_t& lhs, const left_t& rhs) const {
    return !left_.compare(lhs, rhs) && !left_.compare(rhs, lhs);
  }

  bool equal_right(const right_t& lhs, const right_t& rhs) const {
    return !right_.compare(lhs, rhs) && !right_.compare(rhs, lhs);
  }

public:
  friend bool operator==(const bimap& lhs, const bimap& rhs) {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    left_iterator lhs_it_left = lhs.begin_left();
    left_iterator rhs_it_left = rhs.begin_left();
    for (size_t i = 0; i < lhs.size(); ++i, ++lhs_it_left, ++rhs_it_left) {
      if (!lhs.equal_left(*lhs_it_left, *rhs_it_left) || !lhs.equal_right(*lhs_it_left.flip(), *rhs_it_left.flip())) {
        return false;
      }
    }
    return true;
  }

  friend bool operator!=(const bimap& lhs, const bimap& rhs) {
    return !(lhs == rhs);
  }

private:
  mutable sent_t sent_;
  intrusive::bst<left_t, node_t, CompareLeft, left_tag> left_;
  intrusive::bst<right_t, node_t, CompareRight, right_tag> right_;
  size_t size_ = 0;
};

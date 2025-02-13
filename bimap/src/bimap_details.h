#pragma once

#include "bst.h"

namespace details {

class left_tag {};

class right_tag {};

using bst_element_left = intrusive::bst_element<left_tag>;
using bst_element_right = intrusive::bst_element<right_tag>;

struct node_base
    : bst_element_left
    , bst_element_right {
  node_base() = default;
};

template <typename Left, typename Right>
struct node_with_value : node_base {
  node_with_value() = default;

  template <std::convertible_to<Left> L, std::convertible_to<Right> R>
  node_with_value(L&& l, R&& r)
      : left_data_(std::forward<L>(l))
      , right_data_(std::forward<R>(r)) {}

  const Left& get_left() const {
    return left_data_;
  }

  const Right& get_right() const {
    return right_data_;
  }

  template <typename Tag>
  const auto& get_value() const {
    if constexpr (std::is_same_v<Tag, left_tag>) {
      return get_left();
    } else {
      return get_right();
    }
  }

private:
  Left left_data_;
  Right right_data_;
};

template <typename Left, typename Right, typename Compare, typename Tag>
using bst_iterator = typename intrusive::
    bst<std::conditional_t<std::is_same_v<Tag, left_tag>, Left, Right>, node_with_value<Left, Right>, Compare, Tag>::
        iterator;

template <typename Left, typename Right, typename CompareLeft, typename CompareRight>
class bimap_iterator {
public:
  using node_type = node_with_value<Left, Right>;

  template <typename T, typename Compare, typename Tag>
  class iterator;

  using left_iterator = iterator<Left, CompareLeft, left_tag>;
  using right_iterator = iterator<Right, CompareRight, right_tag>;

  template <typename T, typename Compare, typename Tag>
  class iterator : public bst_iterator<Left, Right, Compare, Tag> {
  public:
    using bst_iterator<Left, Right, Compare, Tag>::bst_iterator;
    using base_iterator = bst_iterator<Left, Right, Compare, Tag>;

    using flip_iterator = std::conditional_t<std::is_same_v<Tag, left_tag>, right_iterator, left_iterator>;
    using flip_tag = std::conditional_t<std::is_same_v<Tag, left_tag>, right_tag, left_tag>;

    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = const T&;
    using pointer = const T*;
    using iterator_category = std::bidirectional_iterator_tag;

    iterator(const base_iterator& other)
        : base_iterator(other) {}

    iterator& operator++() {
      base_iterator::operator++();
      return *this;
    }

    iterator operator++(int) {
      iterator temp = *this;
      base_iterator::operator++(0);
      return temp;
    }

    iterator& operator--() {
      base_iterator::operator--();
      return *this;
    }

    iterator operator--(int) {
      iterator temp = *this;
      base_iterator::operator--(0);
      return temp;
    }

    pointer operator->() const {
      return &base_iterator::operator*().template get_value<Tag>();
    }

    reference operator*() const {
      return base_iterator::operator*().template get_value<Tag>();
    }

    node_type* get_node() const {
      return static_cast<node_type*>(base_iterator::operator->());
    }

    flip_iterator flip() const {
      return {static_cast<intrusive::bst_element<flip_tag>*>(
          static_cast<node_base*>(static_cast<intrusive::bst_element<Tag>*>(this->current))
      )};
    }

    friend bool operator==(const iterator& lhs, const iterator& rhs) {
      return lhs.current == rhs.current;
    }

    friend bool operator!=(const iterator& lhs, const iterator& rhs) {
      return !(lhs == rhs);
    }

    template <typename, typename, typename>
    friend class iterator;
  };
};
} // namespace details

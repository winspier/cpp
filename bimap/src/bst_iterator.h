#pragma once
#include "bst_element.h"

#include <iterator>

namespace intrusive::details {
template <typename Node, typename Tag>
class bst_iterator {
public:
  using value_type = Node;
  using difference_type = std::ptrdiff_t;
  using reference = Node&;
  using pointer = Node*;
  using iterator_category = std::bidirectional_iterator_tag;
  using node_pointer = bst_element_base*;

  bst_iterator() = default;

public:
  template <typename, typename, typename>
  friend class bst;

  bst_iterator(node_pointer current)
      : current(current) {}

public:
  pointer operator->() const {
    return static_cast<pointer>(static_cast<bst_element<Tag>*>(current));
  }

  reference operator*() const {
    return *static_cast<pointer>(static_cast<bst_element<Tag>*>(current));
  }

  bst_iterator& operator++() {
    if (current->right_ != nullptr) {
      current = goto_min(current->right_);
      return *this;
    }

    while (current == current->parent_->right_) {
      current = current->parent_;
    }
    current = current->parent_;
    return *this;
  }

  bst_iterator operator++(int) {
    bst_iterator tmp = *this;
    ++*this;
    return tmp;
  }

  bst_iterator& operator--() {
    if (current->left_ != nullptr) {
      current = goto_max(current->left_);
      return *this;
    }

    while (current->parent_ != current && current == current->parent_->left_) {
      current = current->parent_;
    }
    current = current->parent_;
    return *this;
  }

  bst_iterator operator--(int) {
    bst_iterator tmp = *this;
    --*this;
    return tmp;
  }

  bool is_end() const noexcept {
    return current == current->parent_;
  }

  friend bool operator==(const bst_iterator& lhs, const bst_iterator& rhs) {
    return lhs.current == rhs.current;
  }

  friend bool operator!=(const bst_iterator& lhs, const bst_iterator& rhs) {
    return !(lhs == rhs);
  }

public:
  node_pointer current;
};

} // namespace intrusive::details

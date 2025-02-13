#pragma once
#include <utility>

namespace intrusive {
template <typename T, typename Node, typename Compare, typename Tag>
class bst;

class default_tag {};

namespace details {
class bst_element_base {
  template <typename, typename>
  friend class bst_iterator;

  template <typename T, typename Node, typename Compare, typename Tag>
  friend class intrusive::bst;

public:
  bst_element_base() noexcept = default;

protected:
  ~bst_element_base() noexcept = default;

public:
  bool is_linked() const noexcept {
    return parent_ != this;
  }

  void unlink() noexcept {
    parent_ = this;
    left_ = nullptr;
    right_ = nullptr;
    depth_ = 1;
  }

  bst_element_base(bst_element_base&& other) noexcept
      : bst_element_base() {
    *this = std::move(other);
  }

  bst_element_base& operator=(bst_element_base&& other) noexcept {
    if (this != &other) {
      parent_ = std::exchange(other.parent_, &other);
      if (parent_ == &other) {
        parent_ = this;
      }

      if (parent_ != this) {
        if (&other == parent_->left_) {
          parent_->left_ = this;
        } else {
          parent_->right_ = this;
        }
      }

      left_ = std::exchange(other.left_, nullptr);
      right_ = std::exchange(other.right_, nullptr);
      if (left_) {
        left_->parent_ = this;
      }
      if (right_) {
        right_->parent_ = this;
      }
    }
    return *this;
  }

  void set_left(bst_element_base* new_left) {
    left_ = new_left;
    if (new_left != nullptr) {
      new_left->parent_ = this;
    }
  }

  void set_right(bst_element_base* new_right) {
    right_ = new_right;
    if (new_right != nullptr) {
      new_right->parent_ = this;
    }
  }

  void link_with_parent(bst_element_base* v) {
    if (parent_->left_ == this) {
      parent_->left_ = v;
    } else {
      parent_->right_ = v;
    }
    if (v) {
      v->parent_ = parent_;
    }
  }

  bst_element_base(const bst_element_base&) noexcept
      : bst_element_base() {}

  bst_element_base& operator=(const bst_element_base& other) noexcept {
    parent_ = other.parent_;
    left_ = other.left_;
    right_ = other.right_;
    depth_ = other.depth_;
    return *this;
  }

  friend bst_element_base* goto_min(bst_element_base* node) noexcept {
    if (!node) {
      return nullptr;
    }

    while (node->left_ != nullptr) {
      node = node->left_;
    }
    return node;
  }

  friend bst_element_base* goto_max(bst_element_base* node) noexcept {
    if (!node) {
      return nullptr;
    }

    while (node->right_ != nullptr) {
      node = node->right_;
    }
    return node;
  }

protected:
  bst_element_base* parent_ = this;
  bst_element_base* left_ = nullptr;
  bst_element_base* right_ = nullptr;
  std::size_t depth_ = 1;
};
} // namespace details

template <typename Tag = default_tag>
class bst_element : public details::bst_element_base {
  template <typename, typename, typename, typename>
  friend class bst;
};

} // namespace intrusive

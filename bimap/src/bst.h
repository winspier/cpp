#pragma once
#include "bst_element.h"
#include "bst_iterator.h"

#include <algorithm>
#include <iterator>
#include <utility>

namespace intrusive {

template <typename T, typename Node, typename Compare, typename Tag = default_tag>
class bst {
  static_assert(std::is_base_of_v<bst_element<Tag>, Node>, "T must derive from bst_element");
  using node = details::bst_element_base;
  using node_pointer = details::bst_element_base*;
  using value_type = T;

public:
  using iterator = details::bst_iterator<Node, Tag>;

private:
  class position {
    enum type_t {
      LEFT_SON,
      RIGHT_SON,
      CURRENT
    };

    position(node* ptr, type_t t)
        : node_ptr(ptr)
        , type(t) {}

  public:
    bool inserted() {
      return type == type_t::CURRENT;
    }

    iterator get_iterator() {
      return {node_ptr};
    }

  private:
    friend class bst;

    node* node_ptr;
    type_t type;
  };

public:
  ~bst() noexcept = default;

  explicit bst(node_pointer parent, Compare&& comp)
      : parent_(parent)
      , comparator_(std::move(comp)) {}

  bst(const bst&) = delete;
  bst& operator=(const bst&) = delete;

  bst(node_pointer parent, bst&& other) noexcept
      : parent_(parent)
      , comparator_(std::move(other.comparator_)) {}

  bst& operator=(bst&& other) noexcept {
    if (&other != this) {
      bst copy(std::move(other));
      swap(*this, copy);
    }
    return *this;
  }

  position find_position(Node& k) const {
    return find_position(root(), to_value(k));
  }

  position find_position(const value_type& k) const {
    return find_position(root(), k);
  }

public:
  iterator insert(position pos, Node& k) noexcept {
    if (pos.inserted()) {
      *to_node_pointer(&k) = std::move(*pos.node_ptr);
      return {to_node_pointer(&k)};
    }
    node* ptr = to_node_pointer(&k);
    if (pos.type == position::LEFT_SON) {
      pos.node_ptr->set_left(ptr);
    } else {
      pos.node_ptr->set_right(ptr);
    }
    balance_up(ptr);
    return {ptr};
  }

  bool empty() const noexcept {
    return root() != nullptr;
  }

  bool compare(const value_type& lhs, const value_type& rhs) const {
    return comparator_(lhs, rhs);
  }

  iterator lower_bound(const value_type& val) const {
    return {lower_bound(root(), val)};
  }

  iterator upper_bound(const value_type& val) const {
    iterator lb = lower_bound(val);
    if (lb != end() && !compare(val, lb->template get_value<Tag>())) {
      return std::next(lb);
    }
    return lb;
  }

  iterator begin() const noexcept {
    if (!root()) {
      return end();
    }
    return {goto_min(root())};
  }

  iterator end() const noexcept {
    return {parent()};
  }

  iterator erase(iterator pos) noexcept {
    iterator next = std::next(pos);

    node* p = pos.current;
    if (p->right_ == nullptr) {
      p->link_with_parent(p->left_);
    } else {
      node* min = goto_min(p->right_);
      min->set_right(remove_min(p->right_));
      min->set_left(p->left_);
      p->link_with_parent(min);
      balance_up(min);
    }
    return next;
  }

  friend void swap(bst& lhs, bst& rhs) noexcept {
    using std::swap;
    swap(lhs.comparator_, rhs.comparator_);
  }

  iterator find(const value_type& v) const {
    auto pos = find_position(v);
    return pos.inserted() ? pos.get_iterator() : end();
  }

  Compare get_comparator() const {
    return comparator_;
  }

private:
  static node_pointer to_node_pointer(Node* p) {
    return static_cast<node*>(static_cast<bst_element<Tag>*>(p));
  }

  static auto& to_value(node* p) noexcept {
    return static_cast<Node*>(static_cast<bst_element<Tag>*>(p))->template get_value<Tag>();
  }

  static auto& to_value(Node& p) noexcept {
    return p.template get_value<Tag>();
  }

  position find_position(node* p, const value_type& k) const {
    if (!p) {
      return {parent(), position::LEFT_SON};
    }
    if (comparator_(k, to_value(p))) {
      if (p->left_) {
        return find_position(p->left_, k);
      }
      return position{p, position::LEFT_SON};
    }
    if (comparator_(to_value(p), k)) {
      if (p->right_) {
        return find_position(p->right_, k);
      }
      return {p, position::RIGHT_SON};
    }
    return {p, position::CURRENT};
  }

  node* lower_bound(node* t, const value_type& x) const {
    node* res = nullptr;
    while (t) {
      if (!compare(to_value(t), x)) {
        res = t;
        t = t->left_;
      } else {
        t = t->right_;
      }
    }
    return res ? res : parent();
  }

  node* remove_min(node* p) noexcept {
    if (p->left_ == nullptr) {
      return p->right_;
    }
    p->set_left(remove_min(p->left_));
    return balance(p);
  }

  void balance_up(node* p) noexcept {
    if (p == nullptr || p == parent()) {
      return;
    }
    node* up = p->parent_;
    if (up->left_ == p) {
      up->set_left(balance(p));
    } else {
      up->set_right(balance(p));
    }
    balance_up(up);
  }

  int32_t get_size(node* p) const noexcept {
    return p ? p->depth_ : 0;
  }

  void recalc(node* t) noexcept {
    if (t == nullptr) {
      return;
    }
    t->depth_ = 1 + std::max(get_size(t->left_), get_size(t->right_));
  }

  node* rotate_right(node* p) noexcept {
    node* q = p->left_;
    p->set_left(q->right_);
    q->set_right(p);
    recalc(p);
    recalc(q);
    return q;
  }

  node* rotate_left(node* p) noexcept {
    node* r = p->right_;
    p->set_right(r->left_);
    r->set_left(p);
    recalc(p);
    recalc(r);
    return r;
  }

  int32_t bfactor(node* p) const noexcept {
    return p ? get_size(p->right_) - get_size(p->left_) : 0;
  }

  node* balance(node* p) noexcept {
    recalc(p);
    if (bfactor(p) == 2) {
      if (bfactor(p->right_) < 0) {
        p->set_right(rotate_right(p->right_));
      }
      return rotate_left(p);
    }
    if (bfactor(p) == -2) {
      if (bfactor(p->left_) > 0) {
        p->set_left(rotate_left(p->left_));
      }
      return rotate_right(p);
    }
    return p;
  }

  node* root() const noexcept {
    return parent()->left_;
  }

  node* parent() const noexcept {
    return parent_;
  }

private:
  node_pointer parent_;
  [[no_unique_address]] Compare comparator_;
};

template <typename T, typename Compare, typename Tag = default_tag>
void swap(bst<T, Compare, Tag>& lhs, bst<T, Compare, Tag>& rhs) noexcept;
} // namespace intrusive

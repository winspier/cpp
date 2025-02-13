#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

namespace intrusive {

class list_element_base {
  template <typename W>
  friend class list_iterator;

  template <typename T, typename Tag>
  friend class list;

public:
  list_element_base() noexcept;

  ~list_element_base() noexcept;

  bool is_linked() const noexcept;

  void unlink() noexcept;

  list_element_base(list_element_base&& other) noexcept;

  list_element_base& operator=(list_element_base&& other) noexcept;

  list_element_base(const list_element_base&) noexcept;

  list_element_base& operator=(const list_element_base& other) noexcept;

  void link_before(list_element_base* node) noexcept;

  friend void extract_range(list_element_base* first, list_element_base* last) noexcept;

  friend void insert_range(list_element_base* first, list_element_base* last, list_element_base* pos) noexcept;

private:
  list_element_base* prev;
  list_element_base* next;
};

void extract_range(list_element_base* first, list_element_base* last) noexcept;
void insert_range(list_element_base* first, list_element_base* last, list_element_base* pos) noexcept;

class default_tag;

template <typename Tag = default_tag>
class list_element : list_element_base {
  template <typename, typename>
  friend class list;
};

template <typename T, typename Tag = default_tag>
class list {
  static_assert(std::is_base_of_v<list_element<Tag>, T>, "T must derive from list_element");
  using node = list_element<Tag>;
  using node_pointer = list_element_base*;

  template <typename R>
  class list_iterator {
  public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = R&;
    using pointer = R*;
    using iterator_category = std::bidirectional_iterator_tag;

    list_iterator() = default;

  private:
    friend class list;

    list_iterator(node_pointer current)
        : current_node_(current) {}

  public:
    operator list_iterator<const R>() const {
      return {current_node_};
    }

    pointer operator->() const {
      return static_cast<pointer>(current_node_);
    }

    reference operator*() const {
      return *static_cast<pointer>(static_cast<list_element<Tag>*>(current_node_));
    }

    list_iterator& operator++() {
      current_node_ = current_node_->next;
      return *this;
    }

    list_iterator operator++(int) {
      list_iterator tmp = *this;
      ++*this;
      return tmp;
    }

    list_iterator& operator--() {
      current_node_ = current_node_->prev;
      return *this;
    }

    list_iterator operator--(int) {
      list_iterator tmp = *this;
      --*this;
      return tmp;
    }

    friend bool operator==(const list_iterator& lhs, const list_iterator& rhs) {
      return lhs.current_node_ == rhs.current_node_;
    }

    friend bool operator!=(const list_iterator& lhs, const list_iterator& rhs) {
      return !(lhs == rhs);
    }

  private:
    node_pointer current_node_;
  };

public:
  using iterator = list_iterator<T>;
  using const_iterator = list_iterator<const T>;

public:
  // O(1)
  list() noexcept = default;

  // O(1)
  ~list() = default;

  list(const list&) = delete;
  list& operator=(const list&) = delete;

  // O(1)
  list(list&& other) noexcept = default;

  // O(1)
  list& operator=(list&& other) noexcept = default;

  // O(1)
  bool empty() const noexcept {
    return !sentinel_.is_linked();
  }

  // O(n)
  size_t size() const noexcept {
    return std::distance(begin(), end());
  }

  // O(1)
  T& front() noexcept {
    return *begin();
  }

  // O(1)
  const T& front() const noexcept {
    return *begin();
  }

  // O(1)
  T& back() noexcept {
    return *std::prev(end());
  }

  // O(1)
  const T& back() const noexcept {
    return *std::prev(end());
  }

  // O(1)
  void push_front(T& value) noexcept {
    insert(begin(), value);
  }

  // O(1)
  void push_back(T& value) noexcept {
    insert(end(), value);
  }

  // O(1)
  void pop_front() noexcept {
    erase(begin());
  }

  // O(1)
  void pop_back() noexcept {
    erase(std::prev(end()));
  }

  // O(1)
  void clear() noexcept {
    sentinel_.unlink();
  }

  // O(1)
  iterator begin() noexcept {
    return {sentinel_.next};
  }

  // O(1)
  const_iterator begin() const noexcept {
    return {sentinel_.next};
  }

  // O(1)
  iterator end() noexcept {
    return {&sentinel_};
  }

  // O(1)
  const_iterator end() const noexcept {
    return {const_cast<node*>(&sentinel_)};
  }

  // O(1)
  iterator insert(const_iterator pos, T& value) noexcept {
    pos.current_node_->link_before(static_cast<node*>(&value));
    return {pos.current_node_->prev};
  }

  // O(1)
  iterator erase(const_iterator pos) noexcept {
    iterator res = {pos.current_node_->next};
    pos.current_node_->unlink();
    return res;
  }

  // O(1)
  void splice(const_iterator pos, [[maybe_unused]] list& other, const_iterator first, const_iterator last) noexcept {
    if (first == last) {
      return;
    }
    auto first_pointer = first.current_node_;
    auto end_pointer = std::prev(last).current_node_;
    extract_range(first_pointer, end_pointer);
    insert_range(first_pointer, end_pointer, pos.current_node_);
  }

private:
  node sentinel_;
};

} // namespace intrusive

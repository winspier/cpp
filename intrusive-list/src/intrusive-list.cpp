#include "intrusive-list.h"

namespace intrusive {
list_element_base::list_element_base() noexcept
    : prev(this)
    , next(this) {}

list_element_base::~list_element_base() noexcept {
  unlink();
}

bool list_element_base::is_linked() const noexcept {
  return prev != this;
}

void list_element_base::unlink() noexcept {
  prev->next = next;
  next->prev = prev;
  next = prev = this;
}

list_element_base::list_element_base(list_element_base&& other) noexcept
    : list_element_base() {
  *this = std::move(other);
}

list_element_base& list_element_base::operator=(list_element_base&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  unlink();
  if (other.is_linked()) {
    prev = std::exchange(other.prev, &other);
    next = std::exchange(other.next, &other);
    prev->next = this;
    next->prev = this;
  }
  return *this;
}

list_element_base::list_element_base(const list_element_base&) noexcept
    : list_element_base() {}

list_element_base& list_element_base::operator=(const list_element_base& other) noexcept {
  if (this != &other) {
    unlink();
  }
  return *this;
}

void list_element_base::link_before(list_element_base* node) noexcept {
  if (this == node) {
    return;
  }
  node->unlink();
  node->prev = prev;
  node->next = this;
  prev->next = node;
  prev = node;
}

void extract_range(list_element_base* first, list_element_base* last) noexcept {
  first->prev->next = last->next;
  last->next->prev = first->prev;
}

void insert_range(list_element_base* first, list_element_base* last, list_element_base* pos) noexcept {
  first->prev = pos->prev;
  last->next = pos;

  pos->prev->next = first;
  pos->prev = last;
}
} // namespace intrusive

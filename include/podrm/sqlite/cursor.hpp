#pragma once

#include <podrm/sqlite/detail/cursor.hpp>

#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <utility>

namespace podrm::sqlite {

template <typename T> class Cursor {
public:
  class Iterator;
  class Sentinel {};

  Iterator begin() { return Iterator{this->impl}; }
  Sentinel end() { return Sentinel{}; }

private:
  detail::Cursor impl;

  explicit Cursor(detail::Cursor impl) : impl(std::move(impl)) {}

  friend class Database;
};

template <typename T> class Cursor<T>::Iterator {
public:
  using iterator_category = std::input_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = T *;
  using reference = T &;

  T operator*() const {
    T result;
    assert(this->cursor.get().extract(&result));
    return result;
  }

  Iterator &operator++() {
    this->cursor.get().nextRow();

    return *this;
  }

  Iterator operator++(int) { return ++(*this); }

  friend bool operator==(const Iterator &lhs, const Sentinel /*sentinel*/) {
    return !lhs.cursor.get().valid();
  }

private:
  std::reference_wrapper<detail::Cursor> cursor;

  explicit Iterator(detail::Cursor &cursor) : cursor(cursor) {}

  friend class Cursor<T>;
};

} // namespace podrm::sqlite

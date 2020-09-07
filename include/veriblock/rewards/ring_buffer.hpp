// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_RING_BUFFER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_RING_BUFFER_HPP_

#include <algorithm>
#include <vector>

namespace altintegration {

template <class T>
class ring_buffer;

template <class T, bool isconst = false>
struct ring_iterator {
  using size_type = size_t;
  using reference = typename std::conditional<isconst, T const &, T &>::type;
  using pointer = typename std::conditional<isconst, T const *, T *>::type;
  using vec_pointer = typename std::
      conditional<isconst, std::vector<T> const *, std::vector<T> *>::type;

  friend class ring_buffer<T>;

 public:
  ring_iterator()
      : ptrToBuffer(nullptr), offset(0), index(0), reverse(false) {}
  ring_iterator(const ring_iterator<T, false> &it)
      : ptrToBuffer(it.ptrToBuffer),
        offset(it.offset),
        index(it.index),
        reverse(it.reverse) {}

  reference operator*() {
    if (reverse)
      return (*ptrToBuffer)[(ptrToBuffer->size() + offset - index) %
                            (ptrToBuffer->size())];
    return (*ptrToBuffer)[(offset + index) % (ptrToBuffer->size())];
  }
  reference operator[](size_type index) {
    ring_iterator iter = *this;
    iter.index += index;
    return *iter;
  }
  pointer operator->() { return &(operator*()); }

  ring_iterator &operator++() {
    ++index;
    return *this;
  };
  ring_iterator &operator+=(int n) {
    index += n;
    return *this;
  }
  bool operator==(const ring_iterator &other) {
    return (reverse == other.reverse) &&
           (index + offset == other.index + other.offset);
  }
  bool operator!=(const ring_iterator &other) {
    return !this->operator==(other);
  }

 private:
  vec_pointer ptrToBuffer;
  size_type offset;
  size_type index;
  bool reverse;
};

template <class T>
class ring_buffer {
 public:
  using value_type = T;
  using reference = T &;
  using const_reference = const T &;
  using size_type = typename ring_iterator<T, false>::size_type;
  using circularBuffer = std::vector<value_type>;
  using iterator = ring_iterator<T, false>;
  using const_iterator = ring_iterator<T, true>;

  ring_buffer(size_type size)
      : m_array(size + 1), m_array_size(size + 1), m_head(0), m_tail(0) {}

  reference front() {
    assert(!empty());
    return m_array[m_head];
  }
  const_reference front() const {
    assert(!empty());
    return m_array[m_head];
  }

  reference back() {
    assert(!empty());
    return m_array[prev(m_tail)];
  }
  const_reference back() const {
    assert(!empty());
    return m_array[prev(m_tail)];
  }

  size_type capacity() const { return m_array_size - 1; }

  size_type size() const {
    return (m_tail + m_array_size - m_head) % m_array_size;
  }

  reference operator[](size_type index) const {
    return m_array[(m_head + index) % m_array_size];
  }

  iterator begin() const {
    iterator iter;
    iter.ptrToBuffer = &m_array;
    iter.offset = m_head;
    iter.index = 0;
    iter.reverse = false;
    return iter;
  }

  const_iterator cbegin() const {
    const_iterator iter;
    iter.ptrToBuffer = &m_array;
    iter.offset = m_head;
    iter.index = 0;
    iter.reverse = false;
    return iter;
  }

  iterator rbegin() {
    iterator iter;
    iter.ptrToBuffer = &m_array;
    iter.offset = prev(m_tail);
    iter.index = 0;
    iter.reverse = true;
    return iter;
  }

  const_iterator crbegin() const {
    const_iterator iter;
    iter.ptrToBuffer = &m_array;
    iter.offset = prev(m_tail);
    iter.index = 0;
    iter.reverse = true;
    return iter;
  }

  iterator end() {
    iterator iter;
    iter.ptrToBuffer = &m_array;
    iter.offset = m_head;
    iter.index = size();
    iter.reverse = false;
    return iter;
  }

  const_iterator cend() const {
    const_iterator iter;
    iter.ptrToBuffer = &m_array;
    iter.offset = m_head;
    iter.index = size();
    iter.reverse = false;
    return iter;
  }

  iterator rend() {
    iterator iter;
    iter.ptrToBuffer = &m_array;
    iter.offset = prev(m_tail);
    iter.index = size();
    iter.reverse = true;
    return iter;
  }

  const_iterator crend() const {
    const_iterator iter;
    iter.ptrToBuffer = &m_array;
    iter.offset = prev(m_tail);
    iter.index = size();
    iter.reverse = true;
    return iter;
  }

  void push_back(const value_type &item) {
    m_array[m_tail] = item;
    increment_tail();
    if (m_head == m_tail) increment_head();
  }

  value_type pop_front() {
    assert(!empty());
    reference item = front();
    increment_head();
    return item;
  }

  value_type pop_back() {
    assert(!empty());
    reference item = back();
    m_tail = prev(m_tail);
    return item;
  }

  void clear() {
    m_head = 0;
    m_tail = 0;
  }

  bool empty() const { return m_tail == m_head; }
  bool full() const { return m_head == next(m_tail); }

 private:
  circularBuffer m_array;
  size_type m_head;
  size_type m_tail;
  const size_type m_array_size;

  size_type next(size_type cur) const { return (cur + 1) % m_array_size; }
  size_type prev(size_type cur) const {
    return (cur + m_array_size - 1) % m_array_size;
  }

  void increment_head() { m_head = next(m_head); }
  void increment_tail() { m_tail = next(m_tail); }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_RING_BUFFER_HPP_

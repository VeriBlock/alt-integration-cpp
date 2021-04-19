// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_RING_BUFFER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_RING_BUFFER_HPP_

#include <vector>

namespace altintegration {

//! @private
template <class T>
class ring_buffer {
 public:
  using value_type = T;
  using reference = T &;
  using const_reference = const T &;
  using size_type = size_t;
  using circularBuffer = std::vector<value_type>;

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

  size_type capacity() const {
    assert(m_array_size > 0);
    return m_array_size - 1;
  }

  size_type size() const {
    return (m_tail + m_array_size - m_head) % m_array_size;
  }

  bool empty() const { return m_tail == m_head; }
  bool full() const { return m_head == next(m_tail); }

  const_reference operator[](size_type index) const {
    return m_array[(m_head + index) % m_array_size];
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

 private:
  circularBuffer m_array;
  const size_type m_array_size;
  size_type m_head;
  size_type m_tail;

  size_type next(size_type cur) const { return (cur + 1) % m_array_size; }
  size_type prev(size_type cur) const {
    return (cur + m_array_size - 1) % m_array_size;
  }

  void increment_head() { m_head = next(m_head); }
  void increment_tail() { m_tail = next(m_tail); }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_RING_BUFFER_HPP_

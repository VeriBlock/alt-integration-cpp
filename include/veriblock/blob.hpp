#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOB_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOB_HPP_

#include <algorithm>
#include <array>

#include "veriblock/slice.hpp"
#include "veriblock/strutil.hpp"

namespace VeriBlock {

template <size_t N>
struct Blob {
  typedef uint8_t value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef value_type* iterator;
  typedef const value_type* const_iterator;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  Blob() = default;

  explicit Blob(Slice<const uint8_t> slice) { assign(slice); }

  iterator begin() noexcept { return data_.begin(); }

  const_iterator begin() const noexcept { return data_.begin(); }

  iterator end() noexcept { return data_.end(); }

  const_iterator end() const noexcept { return data_.end(); }

  constexpr size_type size() const noexcept { return N; }

  pointer data() noexcept { return data_.data(); }

  const_pointer data() const noexcept { return data_.data(); }

  std::string toHex() { return HexStr(data_.begin(), data_.end()); }

  bool operator==(const Blob<N>& other) { return data_ == other; }

  Blob<N>& operator=(const Blob<N>& other) {
    this->data_ = other.data_;
    return *this;
  }

  Blob<N>& operator=(const Slice<const uint8_t> slice) {
    assign(slice);
    return *this;
  }

  Blob<N> reverse() {
    Blob<N> ret = data_;
    std::reverse(ret.begin(), ret.end());
    return ret;
  }

 private:
  inline void assign(Slice<const uint8_t> slice) {
    if (slice.size() != N) {
      throw std::invalid_argument("Blob(): invalid slice size");
    }
    std::copy(slice.begin(), slice.end(), data_.begin());
  }

  std::array<uint8_t, N> data_;
};  // namespace VeriBlock

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOB_HPP_

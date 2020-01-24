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
  using iterator       = std::_Array_iterator<value_type, N>;
  using const_iterator = std::_Array_const_iterator<value_type, N>;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  Blob() = default;

  Blob(Slice<const uint8_t> slice) { assign(slice); }
  Blob(const std::vector<uint8_t>& v) { assign(v); }

  Blob(const Blob<N>& other) : data_(other.data_) {}
  Blob(Blob<N>&& other) noexcept : data_(std::move(other.data_)) {}

  iterator begin() noexcept { return data_.begin(); }

  const_iterator begin() const noexcept { return data_.begin(); }

  iterator end() noexcept { return data_.end(); }

  const_iterator end() const noexcept { return data_.end(); }

  static size_type size() noexcept { return N; }

  pointer data() noexcept { return data_.data(); }

  const_pointer data() const noexcept { return data_.data(); }

  std::string toHex() const { return HexStr(data_.begin(), data_.end()); }

  bool operator==(const Blob<N>& other) const noexcept {
    return data_ == other.data_;
  }

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

  std::vector<value_type> asVector() const {
    return std::vector<value_type>{data_.begin(), data_.end()};
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

/// custom gtest printer, which prints Blob of any size as hexstring
template <size_t size>
void PrintTo(const Blob<size>& blob, ::std::ostream* os) {
  *os << blob.toHex();
}

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOB_HPP_

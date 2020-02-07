#ifndef ALT_INTEGRATION_VERIBLOCK_BLOB_HPP_
#define ALT_INTEGRATION_VERIBLOCK_BLOB_HPP_

#include <algorithm>
#include <array>
#include <cstring>

#include "veriblock/slice.hpp"
#include "veriblock/strutil.hpp"

namespace VeriBlock {

template <size_t N>
struct Blob {
  using value_type = uint8_t;
  using storage_t = std::array<value_type, N>;
  using pointer = typename storage_t::pointer;
  using const_pointer = typename storage_t::const_pointer;
  using reference = typename storage_t::reference;
  using const_reference = typename storage_t::const_reference;
  using iterator = typename storage_t::iterator;
  using const_iterator = typename storage_t::const_iterator;
  using size_type = typename storage_t::size_type;
  using difference_type = typename storage_t::difference_type;
  using reverse_iterator = typename storage_t::reverse_iterator;
  using const_reverse_iterator = typename storage_t::const_reverse_iterator;

  Blob() {
    for (size_t i = 0; i < N; i++) {
      data_[i] = 0;
    }
  };

  Blob(Slice<const uint8_t> slice) { assign(slice); }
  Blob(const std::vector<uint8_t>& v) { assign(v); }
  Blob(const std::string& v) { assign(v); }

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

  Blob<N>& operator=(const Blob<N>& other) {
    this->data_ = other.data_;
    return *this;
  }

  Blob<N>& operator=(const Slice<const uint8_t> slice) {
    assign(slice);
    return *this;
  }

  Blob<N> reverse() const {
    Blob<N> ret = *this;
    std::reverse(ret.begin(), ret.end());
    return ret;
  }

  std::vector<value_type> asVector() const {
    return std::vector<value_type>{data_.begin(), data_.end()};
  }

  const value_type& operator[](size_t index) noexcept { return data_[index]; }
  const value_type& operator[](size_t index) const noexcept {
    return data_[index];
  }

  int compareTo(const Blob<N>& b) const {
    for (int i = N - 1; i >= 0; i--) {
      if (data_[i] < b.data_[i]) {
        return -1;
      }
      if (data_[i] > b.data_[i]) {
        return 1;
      }
    }
    return 0;
  }

  friend inline bool operator==(const Blob<N>& a, const Blob<N>& b) {
    return memcmp(a.data_.data(), b.data_.data(), a.size()) == 0;
  }
  friend inline bool operator!=(const Blob<N>& a, const Blob<N>& b) {
    return memcmp(a.data_.data(), b.data_.data(), a.size()) != 0;
  }
  friend inline bool operator>(const Blob<N>& a, const Blob<N>& b) {
    return a.compareTo(b) > 0;
  }
  friend inline bool operator<(const Blob<N>& a, const Blob<N>& b) {
    return a.compareTo(b) < 0;
  }
  friend inline bool operator>=(const Blob<N>& a, const Blob<N>& b) {
    return a.compareTo(b) >= 0;
  }
  friend inline bool operator<=(const Blob<N>& a, const Blob<N>& b) {
    return a.compareTo(b) <= 0;
  }

  template <size_t M>
  Blob<M> trim() const {
    if (N < M) {
      throw std::invalid_argument("Blob(): invalid data size");
    }
    Blob<M> m;
    std::copy(data(), data() + M, m.begin());
    return m;
  }

 protected:
  inline void assign(Slice<const uint8_t> slice) {
    if (slice.size() != N) {
      throw std::invalid_argument("Blob(): invalid slice size");
    }
    std::copy(slice.begin(), slice.end(), data_.begin());
  }

  storage_t data_;
};  // namespace VeriBlock

template <size_t M, size_t N>
Blob<M> trim(const Blob<N>& data) {
  if (data.size() < M) {
    throw std::invalid_argument("Blob(): invalid data size");
  }
  return Blob<M>(std::vector<uint8_t>(data.data(), data.data() + M));
}

/// custom gtest printer, which prints Blob of any size as hexstring
template <size_t size>
void PrintTo(const Blob<size>& blob, ::std::ostream* os) {
  *os << blob.toHex();
}

}  // namespace VeriBlock

namespace std {

template <size_t N>
struct hash<VeriBlock::Blob<N>> {
  size_t operator()(const VeriBlock::Blob<N>& x) const {
    return std::hash<std::string>{}(std::string{x.begin(), x.end()});
  }
};
}  // namespace std

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOB_HPP_

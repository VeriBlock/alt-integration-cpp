// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_BLOB_HPP_
#define ALT_INTEGRATION_VERIBLOCK_BLOB_HPP_

#include <algorithm>
#include <array>
#include <cstring>
#include <string>

#include "veriblock/fmt.hpp"
#include "veriblock/slice.hpp"
#include "veriblock/strutil.hpp"

namespace altintegration {

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

  Blob() { data_.fill(0); };

  Blob(Slice<const uint8_t> slice) {
    data_.fill(0);
    assign(slice);
  }
  Blob(const std::vector<uint8_t>& v) {
    data_.fill(0);
    assign(v);
  }
  explicit Blob(const std::string& hex) {
    data_.fill(0);
    auto data = ParseHex(hex);
    assign(data);
  }
  template <size_t M, typename = typename std::enable_if<M <= N>::type>
  explicit Blob(const Blob<M>& other) {
    data_.fill(0);
    assign(other);
  }

  Blob(const Blob<N>& other) : data_(other.data_) {}

  Blob(Blob<N>&& other) noexcept : data_(std::move(other.data_)) {}

  explicit operator std::vector<uint8_t>() const {
    std::vector<uint8_t> ret(begin(), end());
    return ret;
  }

  iterator begin() noexcept { return data_.begin(); }
  const_iterator begin() const noexcept { return data_.begin(); }
  iterator end() noexcept { return data_.end(); }
  const_iterator end() const noexcept { return data_.end(); }
  reverse_iterator rbegin() noexcept { return data_.rbegin(); }
  const_reverse_iterator rbegin() const noexcept { return data_.rbegin(); }
  reverse_iterator rend() noexcept { return data_.rend(); }
  const_reverse_iterator rend() const noexcept { return data_.rend(); }

  static constexpr size_type size() noexcept { return N; }

  pointer data() noexcept { return data_.data(); }

  const_pointer data() const noexcept { return data_.data(); }

  std::string toHex() const { return HexStr(data_.begin(), data_.end()); }

  static Blob<N> fromHex(const std::string& hex) {
    auto data = ParseHex(hex);
    // Blob should set parsed hex in normal order
    return Blob<N>(data);
  }

  static Blob<N> fromHexStrict(const std::string& hex) {
    if (hex.size() / 2 != N) {
      throw std::invalid_argument(
          fmt::sprintf("bad hex length %d, expected %d", hex.size(), N * 2));
    }
    return fromHex(hex);
  }

  Blob<N>& operator=(const Blob<N>& other) {
    this->data_ = other.data_;
    return *this;
  }

  Blob<N>& operator=(const Slice<const uint8_t> slice) {
    assign(slice);
    return *this;
  }

  Blob<N>& operator=(const std::vector<uint8_t>& vec) {
    *this = Blob<N>(vec);
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

  std::string asString() const {
    return std::string{data_.begin(), data_.end()};
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

  friend bool operator==(const Blob<N>& a, const Blob<N>& b) {
    return memcmp(a.data_.data(), b.data_.data(), a.size()) == 0;
  }
  friend bool operator!=(const Blob<N>& a, const Blob<N>& b) {
    return memcmp(a.data_.data(), b.data_.data(), a.size()) != 0;
  }
  friend bool operator>(const Blob<N>& a, const Blob<N>& b) {
    return a.compareTo(b) > 0;
  }
  friend bool operator<(const Blob<N>& a, const Blob<N>& b) {
    return a.compareTo(b) < 0;
  }
  friend bool operator>=(const Blob<N>& a, const Blob<N>& b) {
    return a.compareTo(b) >= 0;
  }
  friend bool operator<=(const Blob<N>& a, const Blob<N>& b) {
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

  template <size_t M>
  Blob<M> trimLE() const {
    if (N < M) {
      throw std::invalid_argument("Blob(): invalid data size");
    }
    Blob<M> m;
    std::copy(data() + size() - M, data() + size(), m.begin());
    return m;
  }

  uint64_t getLow64() const {
    uint64_t ret = 0;
    memcpy(&ret, &data_[0], sizeof(uint64_t));
    return ret;
  }

  std::string toPrettyString() const {
    return fmt::sprintf("Blob<%llu>(%s)", N, toHex());
  }

 protected:
  inline void assign(Slice<const uint8_t> slice) {
    if (slice.size() > N) {
      throw std::invalid_argument(
          "Blob(): invalid data size: " + std::to_string(slice.size()) + " > " +
          std::to_string(N));
    }
    std::copy(slice.begin(), slice.end(), data_.begin());
  }

  inline void assign(const std::string& str) {
    if (str.size() > N) {
      throw std::invalid_argument(
          "Blob(): invalid data size: " + std::to_string(str.size()) + " > " +
          std::to_string(N));
    }
    std::copy(str.begin(), str.end(), data_.begin());
  }

  template <size_t M>
  inline void assign(const Blob<M>& blob) {
    if (N < M) {
      throw std::invalid_argument("Blob(): invalid blob size");
    }
    std::copy(blob.begin(), blob.end(), data_.begin());
  }

  storage_t data_;
};

/// custom gtest printer, which prints Blob of any size as hexstring
template <size_t size>
void PrintTo(const Blob<size>& blob, ::std::ostream* os) {
  *os << blob.toHex();
}

template <typename Value, size_t N>
inline Value ToJSON(const Blob<N>& blob) {
  return ToJSON<Value>(blob.toHex());
}

}  // namespace altintegration

namespace std {

template <size_t N>
struct hash<altintegration::Blob<N>> {
  size_t operator()(std::true_type, const altintegration::Blob<N>& x) const {
    return x.getLow64();
  }

  size_t operator()(std::false_type, const altintegration::Blob<N>& x) const {
    return std::hash<std::string>{}(std::string(x.begin(), x.end()));
  }

  size_t operator()(const altintegration::Blob<N>& x) const {
    // if N >= 8, use getLow64 impl
    // else use std::string hasher
    typename std::conditional<N >= 8, std::true_type, std::false_type>::type f;
    return operator()(f, x);
  }
};
}  // namespace std

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOB_HPP_

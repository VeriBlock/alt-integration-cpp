// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_WRITE_STREAM_HPP
#define ALT_INTEGRATION_VERIBLOCK_WRITE_STREAM_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace altintegration {

/**
 * Binary writer that is useful during binary serialization.
 */
class WriteStream {
 public:
  WriteStream() = default;

  using storage_t = std::vector<uint8_t>;

  explicit WriteStream(size_t size) { m_data.reserve(size); }

  void write(const void *buf, size_t size) {
    const uint8_t *inp = (uint8_t *)buf;
    m_data.insert(m_data.end(), inp, inp + size);
  }

  template <typename T,
            typename = typename std::enable_if<sizeof(typename T::value_type) ==
                                               1>::type>
  void write(const T &t) {
    write(t.data(), t.size());
  }

  template <
      typename T,
      typename = typename std::enable_if<std::is_integral<T>::value>::type>
  void writeBE(T num) {
    for (size_t i = 0, shift = (sizeof(T) - 1) * 8; i < sizeof(T);
         i++, shift -= 8) {
      m_data.push_back((num >> shift) & 0xffu);
    }
  }

  template <
      typename T,
      typename = typename std::enable_if<std::is_integral<T>::value>::type>
  void writeLE(T num) {
    for (size_t i = 0, shift = 0; i < sizeof(T); i++, shift += 8) {
      m_data.push_back((num >> shift) & 0xffu);
    }
  }

  const storage_t &data() const noexcept { return m_data; }

 private:
  storage_t m_data;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_WRITE_STREAM_HPP

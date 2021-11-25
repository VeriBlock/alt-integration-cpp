// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_WRITE_STREAM_HPP
#define ALT_INTEGRATION_VERIBLOCK_WRITE_STREAM_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <veriblock/pop/assert.hpp>

namespace altintegration {

/**
 * Binary writer that is useful for binary serialization.
 */
class WriteStream {
 public:
  WriteStream() = default;

  using storage_t = std::vector<uint8_t>;

  explicit WriteStream(size_t size);

  // movable
  explicit WriteStream(WriteStream &&) = default;
  WriteStream &operator=(WriteStream &&) = default;

  void write(const void *buf, size_t size);

  template <typename T,
            typename = typename std::enable_if<sizeof(typename T::value_type) ==
                                               1>::type>
  void write(const T &t) {
    write(t.data(), t.size());
  }

  template <
      typename T,
      typename = typename std::enable_if<std::is_integral<T>::value>::type>
  void writeBE(T num, size_t bytes = sizeof(T)) {
    VBK_ASSERT(bytes <= sizeof(T));

    for (size_t i = 0, shift = (sizeof(T) - 1) * 8; i < sizeof(T);
         i++, shift -= 8) {
      // skip first bytes
      if (i < (sizeof(T) - bytes)) {
        continue;
      }
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

  uint32_t getVersion() const noexcept;

  void setVersion(uint32_t version) noexcept;

  const storage_t &data() const noexcept;

  std::string hex() const noexcept;

 private:
  // publicly non-copyable
  WriteStream(const WriteStream &) = default;
  WriteStream &operator=(const WriteStream &) = default;

 private:
  uint32_t m_version{0};
  storage_t m_data;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_WRITE_STREAM_HPP

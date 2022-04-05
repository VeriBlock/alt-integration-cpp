// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_READ_STREAM_HPP
#define ALT_INTEGRATION_VERIBLOCK_READ_STREAM_HPP

#include <stdexcept>
#include <type_traits>
#include <vector>

#include "assert.hpp"
#include "slice.hpp"
#include "validation_state.hpp"

namespace altintegration {

/**
 * Binary reading stream, that is useful during binary deserialization.
 */
struct ReadStream {
 public:
  ReadStream(const void *buff, size_t size);

  explicit ReadStream(Slice<const uint8_t> slice);

  explicit ReadStream(const std::vector<uint8_t> &v);

  explicit ReadStream(const std::string &s);

  // movable
  explicit ReadStream(ReadStream &&) = default;
  ReadStream &operator=(ReadStream &&) = default;

  /**
   * Read type T of 'size' bytes
   * @param size bytes to be read
   * @param out preallocated array of at least `size`
   * @param state will return error description here
   * @return true if read is OK, false otherwise
   */
  bool read(size_t size, uint8_t *out, ValidationState &state);

  std::vector<uint8_t> assertRead(size_t size);

  bool readSlice(size_t size,
                 Slice<const uint8_t> &out,
                 ValidationState &state);

  Slice<const uint8_t> assertReadSlice(size_t size);

  template <
      typename T,
      typename = typename std::enable_if<std::is_integral<T>::value>::type>
  T assertReadBE(size_t bytes = sizeof(T)) {
    ValidationState state;
    T t = 0;
    bool result = readBE<T>(t, state, bytes);
    VBK_ASSERT_MSG(result, "Can't readBE: %s", state.toString());
    return t;
  }

  // big endian
  template <
      typename T,
      typename = typename std::enable_if<std::is_integral<T>::value>::type>
  bool readBE(T &t, ValidationState &state, size_t bytes = sizeof(T)) {
    if (!hasMore(bytes)) {
      return state.Invalid(
          "readbe-underflow",
          format("Tried to read {} bytes from stream, but it has {} bytes",
                 bytes,
                 remaining()));
    }
    t = 0;
    for (size_t i = 0, shift = (bytes - 1) * 8; i < bytes; i++, shift -= 8) {
      t += static_cast<T>(((T)m_Buffer[m_Pos++]) << shift);
    }
    return true;
  }

  template <
      typename T,
      typename = typename std::enable_if<std::is_integral<T>::value>::type>
  T assertReadLE() {
    ValidationState state;
    T t = 0;
    bool result = readLE<T>(t, state);
    VBK_ASSERT_MSG(result, "Can't readLE: %s", state.toString());
    return t;
  }

  // little endian
  template <
      typename T,
      typename = typename std::enable_if<std::is_integral<T>::value>::type>
  bool readLE(T &t, ValidationState &state) {
    if (!hasMore(sizeof(T))) {
      return state.Invalid(
          "readle-underflow",
          format("Tried to read {} bytes from stream, but it has {} bytes",
                 sizeof(T),
                 remaining()));
    }

    t = 0;
    for (size_t i = 0, shift = 0; i < sizeof(T); i++, shift += 8) {
      t += static_cast<T>(((T)m_Buffer[m_Pos++]) << shift);
    }
    return true;
  }

  uint32_t getVersion() const noexcept;
  void setVersion(uint32_t version) noexcept;
  size_t position() const noexcept;
  void setPosition(const size_t &) noexcept;
  size_t remaining() const noexcept { return (m_Size - m_Pos); }
  bool hasMore(size_t nbytes) const noexcept { return (remaining() >= nbytes); }
  void reset() noexcept;
  Slice<const uint8_t> data() const;
  Slice<const uint8_t> remainingBytes() const;

 private:
  void assign(const void *buff, size_t numOfBytes);

  // publicly non-copyable
  ReadStream(const ReadStream &) = default;
  ReadStream &operator=(const ReadStream &) = default;

 private:
  uint32_t m_version{0};
  size_t m_Pos{0};
  const uint8_t *m_Buffer{nullptr};
  size_t m_Size{0};
};

}  // namespace altintegration

#endif

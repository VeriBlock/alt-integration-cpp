// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_READ_STREAM_HPP
#define ALT_INTEGRATION_VERIBLOCK_READ_STREAM_HPP

#include <stdexcept>
#include <type_traits>
#include <vector>

#include "slice.hpp"
#include "validation_state.hpp"

namespace altintegration {

/**
 * Binary reading stream, that is useful during binary deserialization.
 */
class ReadStream {
 public:
  ReadStream(const void *buff, size_t size);

  explicit ReadStream(Slice<const uint8_t> slice);

  explicit ReadStream(const std::vector<uint8_t> &v);

  explicit ReadStream(const std::string &s);

  /**
   * Read vector of 'size' bytes
   * @param size bytes to be read
   * @param out vector of bytes read from a stream
   * @param state will return error description here
   * @return true if read is OK, false otherwise
   */
  bool read(size_t size, std::vector<uint8_t> &out, ValidationState &state);

  /**
   * Read vector of 'size' bytes
   * @param size bytes to be read
   * @return vector of bytes read from a stream
   */
  std::vector<uint8_t> read(size_t size);

  /**
   * Read type T of 'size' bytes
   * @tparam T any contiguous storage type (std::string, std::vector)
   * @param size bytes to be read
   * @param out T of bytes read from a stream
   * @param state will return error description here
   * @return true if read is OK, false otherwise
   */
  template <typename T,
            typename = typename std::enable_if<sizeof(typename T::value_type) ==
                                               1>::type>
  bool read(size_t size, T &out, ValidationState &state) {
    if (!hasMore(size)) {
      return state.Invalid("read-buffer-underflow");
    }

    T result;
    result.resize(size);
    std::copy(m_Buffer + m_Pos, m_Buffer + m_Pos + size, result.data());
    m_Pos += size;
    out = result;
    return true;
  }

  /**
   * Read type T of 'size' bytes
   * @tparam T any contiguous storage type (std::string, std::vector)
   * @param size bytes to be read
   * @return T of bytes read from a stream
   */
  template <typename T,
            typename = typename std::enable_if<sizeof(typename T::value_type) ==
                                               1>::type>
  T read(size_t size) {
    T out;
    ValidationState state;
    if (!read(size, out, state)) {
      throw std::out_of_range("stream.read(): out of data");
    }
    return out;
  }

  bool readSlice(size_t size,
                 Slice<const uint8_t> &out,
                 ValidationState &state);

  Slice<const uint8_t> readSlice(size_t size);

  // big endian
  template <
      typename T,
      typename = typename std::enable_if<std::is_integral<T>::value>::type>
  bool readBE(T &out, ValidationState &state) {
    if (!hasMore(sizeof(T))) {
      return state.Invalid("readbe-buffer-underflow");
    }
    T t = 0;
    for (size_t i = 0, shift = (sizeof(T) - 1) * 8; i < sizeof(T);
         i++, shift -= 8) {
      t += ((T)m_Buffer[m_Pos++]) << shift;
    }
    out = t;
    return true;
  }

  // big endian
  template <
      typename T,
      typename = typename std::enable_if<std::is_integral<T>::value>::type>
  T readBE() {
    T out;
    ValidationState state;
    if (!readBE(out, state)) {
      throw std::out_of_range("stream.readBE(): out of data");
    }
    return out;
  }

  // little endian
  template <
      typename T,
      typename = typename std::enable_if<std::is_integral<T>::value>::type>
  bool readLE(T &out, ValidationState &state) {
    if (!hasMore(sizeof(T))) {
      return state.Invalid("readle-buffer-underflow");
    }

    T t = 0;
    for (size_t i = 0, shift = 0; i < sizeof(T); i++, shift += 8) {
      t += m_Buffer[m_Pos++] << shift;
    }
    out = t;
    return true;
  }

  // little endian
  template <
      typename T,
      typename = typename std::enable_if<std::is_integral<T>::value>::type>
  T readLE() {
    T out;
    ValidationState state;
    if (!readLE(out, state)) {
      throw std::out_of_range("stream.readLE(): out of data");
    }
    return out;
  }

  size_t position() const noexcept;
  void setPosition(const size_t &) noexcept;
  size_t remaining() const noexcept;
  bool hasMore(size_t nbytes) const noexcept;
  void reset() noexcept;
  Slice<const uint8_t> data() const;

 private:
  void assign(const void *buff, size_t numOfBytes);

 private:
  size_t m_Pos = 0;
  const uint8_t *m_Buffer = nullptr;
  size_t m_Size = 0;
};

}  // namespace altintegration

#endif

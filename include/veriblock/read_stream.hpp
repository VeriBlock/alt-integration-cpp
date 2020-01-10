#ifndef VERIBLOCK_READ_STREAM_H_
#define VERIBLOCK_READ_STREAM_H_

#include <stdexcept>
#include <type_traits>
#include <vector>

#include "slice.hpp"

namespace VeriBlock {

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
   * @return vector of bytes read from a stream
   */
  std::vector<uint8_t> read(size_t size);

  /**
   * Read type T of 'size' bytes
   * @tparam T any contiguous storage type (std::string, std::vector)
   * @param size bytes to be read
   * @return T of bytes raad from a stream
   */
  template <typename T,
            typename = typename std::enable_if<sizeof(typename T::value_type) ==
                                               1>::type>
  T read(size_t size) {
    if (!hasMore(size)) {
      throw std::out_of_range("stream.read(): out of data");
    }

    T result;
    result.resize(size);
    std::copy(m_Buffer + m_Pos, m_Buffer + m_Pos + size, result.data());
    m_Pos += size;
    return result;
  }

  Slice<const uint8_t> readSlice(size_t size);

  // big endian
  template <
      typename T,
      typename = typename std::enable_if<std::is_integral<T>::value>::type>
  T readBE() {
    if (!hasMore(sizeof(T))) {
      throw std::out_of_range("stream.readSingleBEValue(): out of data");
    }
    T t = 0;
    for (size_t i = 0, shift = (sizeof(T) - 1) * 8; i < sizeof(T);
         i++, shift -= 8) {
      t += m_Buffer[m_Pos++] << shift;
    }
    return t;
  }

  //  little endian
  template <
      typename T,
      typename = typename std::enable_if<std::is_integral<T>::value>::type>
  T readLE() {
    if (!hasMore(sizeof(T))) {
      throw std::out_of_range("stream.readLE(): out of data");
    }
    T t = 0;
    for (size_t i = 0, shift = 0; i < sizeof(T); i++, shift += 8) {
      t += m_Buffer[m_Pos++] << shift;
    }
    return t;
  }

  size_t position() const noexcept;
  size_t remaining() const noexcept;
  bool hasMore(size_t nbytes) const noexcept;
  void reset() noexcept;

 private:
  void assign(const void *buff, size_t numOfBytes);

 private:
  size_t m_Pos = 0;
  const uint8_t *m_Buffer = nullptr;
  size_t m_Size = 0;
};

}  // namespace VeriBlock

#endif

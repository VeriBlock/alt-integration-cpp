#ifndef ALT_INTEGRATION_SERDE_HPP
#define ALT_INTEGRATION_SERDE_HPP

#include <functional>
#include <limits>
#include <stdexcept>
#include <vector>

#include "blob.hpp"
#include "consts.hpp"
#include "read_stream.hpp"
#include "slice.hpp"

/**
 * Contains veriblock-specific serialization and deserialziation primitives.
 */

namespace VeriBlock {

/**
 * Checks if expression 'min' <= 'num' <= 'max' is true. If false, throws.
 * @param num number to check
 * @param min min value
 * @param max max value
 * @throws std::out_of_range
 */
void checkRange(int64_t num, int64_t min, int64_t max);

/**
 * Pad container 'v' to have size at least 'size', by adding leading zeroes
 * @tparam T input container type
 * @param v input container
 * @param size output vector will have at least 'size' bytes
 * @return new vector with padded data
 */
template <typename T>
std::vector<uint8_t> pad(const T& v, size_t size) {
  if (v.size() > size) {
    return std::vector<uint8_t>{v.begin(), v.end()};
  }

  std::vector<uint8_t> ret(size, 0);
  // get absolute value of difference between size and v.size()
  const auto diff = size > v.size() ? size - v.size() : v.size() - size;
  std::copy(v.begin(), v.end(), ret.begin() + diff);
  return ret;
}

/**
 * Read variable length value, which consists of
 * `[N=(4 bytes = size of slice) | N bytes slice]`
 * Size of slice should be within range [minLen; maxLen]
 * @param stream read data from this stream
 * @param minLen minimum possible value of slice size
 * @param maxLen maximum possible value of slice size
 * @throws std::out_of_range if size is out of range or stream is out of data
 * @return slice with data
 */
Slice<const uint8_t> readVarLenValue(
    ReadStream& stream,
    int32_t minLen = 0,
    int32_t maxLen = std::numeric_limits<int32_t>::max());

/**
 * Read variable length value, which consists of
 * `[N=(1 byte = size of slice) | N bytes slice]`
 * Size of slice should be within range [minLen; maxLen]
 * @param stream read data from this stream
 * @param minLen minimum possible value of slice size
 * @param maxLen maximum possible value of slice size
 * @throws std::out_of_range if size is out of range or stream is out of data
 * @return slice with data
 */
Slice<const uint8_t> readSingleByteLenValue(
    ReadStream& stream,
    int32_t minLen = 0,
    int32_t maxLen = std::numeric_limits<int32_t>::max());

/**
 * Read single Big-Endian value from a stream.
 * This function interprets sizeof(T) bytes as Big-Endian number and returns it.
 * @tparam T number type - uint64_t, etc.
 * @param stream read data from this stream
 * @throws std::out_of_range if stream is out of data
 * @return read number
 */
template <typename T,
          typename = typename std::enable_if<std::is_integral<T>::value>::type>
T readSingleBEValue(ReadStream& stream) {
  return ReadStream(
             pad(readSingleByteLenValue(stream, 0, sizeof(T)), sizeof(T)))
      .readBE<T>();
}

struct NetworkBytePair {
  ///< works as std::optional. if hasNetworkByte is true, networkByte is set
  bool hasNetworkByte = false;
  uint8_t networkByte = 0;
  uint8_t typeId = 0;
};

NetworkBytePair readNetworkByte(ReadStream& stream, TxType type);

/**
 * Reads array of entities of type T.
 * @tparam T type of entity to read
 * @param stream read data from this stream
 * @param min min size of array
 * @param max max size of array
 * @param readFunc function that is called to read single value of type T
 * @throws std::out_of_range if stream is out of data
 * @return vector of read elements of type T
 */
template <typename T>
std::vector<T> readArrayOf(ReadStream& stream,
                           int32_t min,
                           int32_t max,
                           std::function<T(ReadStream&)> readFunc) {
  const auto count = readSingleBEValue<int32_t>(stream);
  checkRange(count, min, max);

  std::vector<T> items;
  items.reserve(count);

  for (int32_t i = 0; i < count; i++) {
    items.push_back(readFunc(stream));
  }

  return items;
}

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_SERDE_HPP

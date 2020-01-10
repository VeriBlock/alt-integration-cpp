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

Slice<const uint8_t> readVarLenValue(
    ReadStream& stream,
    int32_t minLen = 0,
    int32_t maxLen = std::numeric_limits<int32_t>::max());

Slice<const uint8_t> readSingleByteLenValue(
    ReadStream& stream,
    int32_t minLen = 0,
    int32_t maxLen = std::numeric_limits<int32_t>::max());

template <typename T,
          typename = typename std::enable_if<std::is_integral<T>::value>::type>
T readSingleBEValue(ReadStream& stream, int32_t min, int32_t max) {
  return ReadStream(pad(readSingleByteLenValue(stream, min, max), max))
      .readBE<T>();
}

struct NetworkBytePair {
  ///< works as std::optional. if hasNetworkByte is true, networkByte is set
  bool hasNetworkByte = false;
  uint8_t networkByte = 0;
  uint8_t typeId = 0;
};

NetworkBytePair readNetworkByte(ReadStream& stream, TxType type);

template <typename T>
std::vector<T> readArrayOf(ReadStream& stream,
                           int32_t min,
                           int32_t max,
                           std::function<T(ReadStream&)> readFunc) {
  const auto count = readSingleBEValue<int32_t>(stream, 0, 4);
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

// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_SERDE_HPP
#define ALT_INTEGRATION_VERIBLOCK_SERDE_HPP

#include <functional>
#include <limits>
#include <stdexcept>
#include <vector>
#include <veriblock/assert.hpp>

#include "checks.hpp"
#include "consts.hpp"
#include "read_stream.hpp"
#include "slice.hpp"
#include "write_stream.hpp"

/**
 * Contains veriblock-specific serialization and deserialziation primitives.
 */

namespace altintegration {

/**
 * Converts the input to the byte array and trims it's size to the
 * lowest possible value
 * @param input value to convert
 * @return converted and trimmed byte array
 */
std::vector<uint8_t> trimmedArray(int64_t input);

/**
 * Converts the input to the byte array
 * @param input value to convert
 * @return converted byte array
 */
template <typename T,
          typename = typename std::enable_if<std::is_integral<T>::value>::type>
std::vector<uint8_t> fixedArray(T input) {
  WriteStream inputStream;
  inputStream.writeBE(input);
  return inputStream.data();
}

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
    return std::vector<uint8_t>(v.begin(), v.end());
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
    int32_t maxLen = (std::numeric_limits<int32_t>::max)());

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
    int32_t maxLen = (std::numeric_limits<int32_t>::max)());

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

/**
 * Write single byte length value, which consists of
 * `N bytes vector`
 * Appends 1 byte data length to the stream
 * @param stream write data to this stream
 * @param value data that should be written
 * @throws std::out_of_range if value size is too high
 */
void writeSingleByteLenValue(WriteStream& stream, Slice<const uint8_t> value);

/**
 * Write single Big-Endian value to the stream.
 * This function converts number to the bytes array
 * in Big-Endian order, trims it and writes to the stream
 * @param stream write data to this stream
 * @param value value to be written
 * @throws std::out_of_range if stream is out of data
 */
void writeSingleBEValue(WriteStream& stream, int64_t value);

/**
 * Write single Big-Endian value to the stream.
 * This function converts number to the bytes array
 * in Big-Endian order and writes to the stream
 * @param stream write data to this stream
 * @param value value to be written
 * @throws std::out_of_range if stream is out of data
 */
template <typename T,
          typename = typename std::enable_if<std::is_integral<T>::value>::type>
void writeSingleFixedBEValue(WriteStream& stream, T value) {
  WriteStream dataStream;
  dataStream.writeBE<T>(value);
  writeSingleByteLenValue(stream, dataStream.data());
}

/**
 * Write variable length value, which consists of
 * `N bytes vector`
 * Appends up to 8 bytes data length to the stream and
 * 1 byte data length size
 * @param stream write data to this stream
 * @param value data that should be written
 */
void writeVarLenValue(WriteStream& stream, Slice<const uint8_t> value);

struct NetworkBytePair {
  ///< works as std::optional. if hasNetworkByte is true, networkByte is set
  bool hasNetworkByte = false;
  uint8_t networkByte = 0;
  uint8_t typeId = 0;
};

/**
 * Read optional network byte from the stream
 * @param stream read data from this stream
 * @param type use this value to detect if we are reading
 * network byte or type byte
 * @throws std::out_of_range if stream is out of data
 * @return NetworkBytePair structure
 */
NetworkBytePair readNetworkByte(ReadStream& stream, TxType type);

/**
 * Write optional network byte to the stream
 * @param stream write data to this stream
 * @param networkOrType write network byte if available, write type
 * byte after
 */
void writeNetworkByte(WriteStream& stream, NetworkBytePair networkOrType);

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

template <typename T>
std::vector<T> readArrayOf(ReadStream& stream,
                           std::function<T(ReadStream&)> readFunc) {
  int32_t max = std::numeric_limits<int32_t>::max();
  return readArrayOf<T>(stream, 0, max, readFunc);
}

std::string readString(ReadStream& stream);

void writeDouble(WriteStream& stream, const double& val);

double readDouble(ReadStream& stream);

template <typename Container>
void writeContainer(
    WriteStream& w,
    const Container& t,
    std::function<void(WriteStream& w, const typename Container::value_type& t)>
        f) {
  writeSingleBEValue(w, (int64_t)t.size());
  for (auto& v : t) {
    f(w, v);
  }
}

template <typename T>
void writeArrayOf(WriteStream& w,
                  const std::vector<T>& t,
                  std::function<void(WriteStream& w, const T& t)> f) {
  return writeContainer<std::vector<T>>(w, t, f);
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_SERDE_HPP

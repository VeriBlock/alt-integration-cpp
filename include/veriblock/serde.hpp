// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_SERDE_HPP
#define ALT_INTEGRATION_VERIBLOCK_SERDE_HPP

#include <functional>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>

#include "consts.hpp"
#include "read_stream.hpp"
#include "slice.hpp"
#include "strutil.hpp"
#include "write_stream.hpp"

/**
 * Contains veriblock-specific serialization and deserialziation primitives.
 */

namespace altintegration {

/**
 * Checks if expression 'min' <= 'num' <= 'max' is true. If false, returns
 * invalid state with error description.
 * @param num number to check
 * @param min min value
 * @param max max value
 * @param state will return error description here
 * @return true if check is OK, false otherwise
 */
bool checkRange(int64_t num, int64_t min, int64_t max, ValidationState& state);

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
 * Read variable length value, which consists of
 * `[N=(4 bytes = size of slice) | N bytes slice]`
 * Size of slice should be within range [minLen; maxLen]
 * @param stream read data from this stream
 * @param out slice with data
 * @param state will return error description here
 * @param minLen minimum possible value of slice size
 * @param maxLen maximum possible value of slice size
 * @return true if read is OK, false otherwise
 */
bool readVarLenValue(ReadStream& stream,
                     Slice<const uint8_t>& out,
                     ValidationState& state,
                     int32_t minLen = 0,
                     int32_t maxLen = (std::numeric_limits<int32_t>::max)());

/**
 * Read variable length value, which consists of
 * `[N=(1 byte = size of slice) | N bytes slice]`
 * Size of slice should be within range [minLen; maxLen]
 * @param stream read data from this stream
 * @param out slice with data
 * @param state will return error description here
 * @param minLen minimum possible value of slice size
 * @param maxLen maximum possible value of slice size
 * @return true if read is OK, false otherwise
 */
bool readSingleByteLenValue(ReadStream& stream,
                            Slice<const uint8_t>& out,
                            ValidationState& state,
                            int minLen,
                            int maxLen);

//! @overload
template <typename Container,
          typename = typename std::enable_if<
              sizeof(typename Container::value_type) == 1>::type>
bool readSingleByteLenValue(ReadStream& stream,
                            Container& out,
                            ValidationState& state,
                            int minLen,
                            int maxLen) {
  uint8_t length = 0;
  if (!stream.readBE<uint8_t>(length, state)) {
    return state.Invalid("readsingle-bad-length");
  }
  if (!checkRange(length, minLen, maxLen, state)) {
    return state.Invalid("readsingle-bad-range");
  }
  out.resize(length);
  return stream.read(length, out.data(), state);
}

/**
 * Read single Big-Endian value from a stream.
 * This function interprets sizeof(T) bytes as Big-Endian number and returns
 * it.
 * @tparam T number type - uint64_t, etc.
 * @param stream read data from this stream
 * @param out read number
 * @param state will return error description here
 * @return true if read is OK, false otherwise
 */
template <typename T,
          typename = typename std::enable_if<std::is_integral<T>::value>::type>
bool readSingleBEValue(ReadStream& stream, T& out, ValidationState& state) {
  Slice<const uint8_t> data;
  if (!readSingleByteLenValue(stream, data, state, 0, sizeof(T))) {
    return state.Invalid("readsinglebe-bad-data");
  }
  auto dataStream = ReadStream(data);
  return dataStream.readBE<T>(out, state, data.size());
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

/**
 * Stores pair of TxType and VBK network byte.
 */
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
 * Read optional network byte from the stream
 * @param stream read data from this stream
 * @param type use this value to detect if we are reading
 * network byte or type byte
 * @param out NetworkBytePair structure
 * @param state will return error description here
 * @return true if read is OK, false otherwise
 */
bool readNetworkByte(ReadStream& stream,
                     TxType type,
                     NetworkBytePair& out,
                     ValidationState& state);

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
 * @param out vector of read elements of type T
 * @param state will return error description here
 * @param min min size of array
 * @param max max size of array
 * @param readFunc function that is called to read single value of type T
 * @return true if read is OK, false otherwise
 */
template <typename T>
bool readArrayOf(ReadStream& stream,
                 std::vector<T>& out,
                 ValidationState& state,
                 int32_t min,
                 int32_t max,
                 std::function<bool(T&)> readFunc) {
  int32_t count = 0;
  if (!readSingleBEValue<int32_t>(stream, count, state)) {
    return state.Invalid("readarray-bad-count");
  }
  if (!checkRange(count, min, max, state)) {
    return state.Invalid("readarray-bad-range");
  }

  std::vector<T> items;
  items.reserve(count);

  for (int32_t i = 0; i < count; i++) {
    T item;
    if (!readFunc(item)) {
      return state.Invalid("readarray-bad-item", i);
    }
    items.push_back(item);
  }

  out = items;
  return true;
}

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
bool readArrayOf(ReadStream& stream,
                 std::vector<T>& out,
                 ValidationState& state,
                 std::function<bool(T&)> readFunc) {
  int32_t max = std::numeric_limits<int32_t>::max();
  return readArrayOf<T>(stream, out, state, 0, max, readFunc);
}

template <typename Container>
void writeContainer(
    WriteStream& w,
    const Container& t,
    std::function<void(WriteStream&, const typename Container::value_type& t)>
        f) {
  writeSingleBEValue(w, (int64_t)t.size());
  for (auto& v : t) {
    f(w, v);
  }
}

template <typename T>
void writeArrayOf(WriteStream& w,
                  const std::vector<T>& t,
                  std::function<void(WriteStream&, const T& t)> f) {
  return writeContainer<std::vector<T>>(w, t, f);
}

template <typename T>
bool DeserializeFromVbkEncoding(Slice<const uint8_t> data,
                                T& out,
                                ValidationState& state) {
  ReadStream stream(data);
  return DeserializeFromVbkEncoding(stream, out, state);
}

template <typename T>
bool DeserializeFromRaw(Slice<const uint8_t> data,
                        T& out,
                        ValidationState& state) {
  ReadStream stream(data);
  return DeserializeFromRaw(stream, out, state);
}

template <typename T>
bool DeserializeFromHex(const std::string& hex,
                        T& out,
                        ValidationState& state) {
  auto data = ParseHex(hex);
  ReadStream stream(data);
  return DeserializeFromVbkEncoding(stream, out, state);
}

template <typename T>
std::vector<uint8_t> SerializeToVbkEncoding(const T& obj) {
  WriteStream w;
  obj.toRaw(w);
  return w.data();
}

template <typename T>
std::vector<uint8_t> SerializeToRaw(const T& obj) {
  WriteStream w;
  obj.toRaw(w);
  return w.data();
}

template <typename T>
std::string SerializeToHex(const T& obj) {
  return HexStr(SerializeToRaw<T>(obj));
}

template <typename T>
T AssertDeserializeFromRaw(std::vector<uint8_t> raw) {
  T t;
  ValidationState state;
  bool result = DeserializeFromRaw(raw, t, state);
  VBK_ASSERT_MSG(result, "Can't deserialize: %s", state.toString());
  return t;
}

template <typename T>
T AssertDeserializeFromVbkEncoding(Slice<const uint8_t> raw) {
  T t;
  ValidationState state;
  bool result = DeserializeFromVbkEncoding(raw, t, state);
  VBK_ASSERT_MSG(result, "Can't deserialize: %s", state.toString());
  return t;
}

template <typename T>
T AssertDeserializeFromHex(std::string hex) {
  T t;
  ValidationState state;
  bool result = DeserializeFromHex(hex, t, state);
  VBK_ASSERT_MSG(result, "Can't deserialize: %s", state.toString());
  return t;
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_SERDE_HPP

// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>
#include <veriblock/serde.hpp>

namespace altintegration {

std::vector<uint8_t> trimmedArray(int64_t input) {
  size_t x = sizeof(int64_t);
  do {
    if ((input >> ((x - 1) * 8)) != 0) {
      break;
    }
    x--;
  } while (x > 1);

  std::vector<uint8_t> output(x);
  for (size_t i = 0; i < x; i++) {
    output[x - i - 1] = (uint8_t)input;
    input >>= 8;
  }

  return output;
}

Slice<const uint8_t> readVarLenValue(ReadStream& stream,
                                     int minLen,
                                     int maxLen) {
  auto length = readSingleBEValue<int32_t>(stream);
  checkRange(length, minLen, maxLen);
  return stream.readSlice(length);
}

bool readVarLenValueNoExcept(
  ReadStream& stream,
  Slice<const uint8_t>& out,
    ValidationState& state,
    int32_t minLen,
    int32_t maxLen) {
  int32_t length;
  if (!readSingleBEValueNoExcept<int32_t>(stream, length, state)) {
    return state.Invalid("malformed-length");
  }
  if (!checkRangeNoExcept(length, minLen, maxLen, state)) {
    return state.Invalid("bad-length");
  }
  return stream.readSliceNoExcept(length, out, state);

}

Slice<const uint8_t> readSingleByteLenValue(ReadStream& stream,
                                            int minLen,
                                            int maxLen) {
  const auto lengthLength = stream.readBE<uint8_t>();
  checkRange(lengthLength, minLen, maxLen);
  return stream.readSlice(lengthLength);
}

bool readSingleByteLenValueNoExcept(ReadStream& stream,
                                    Slice<const uint8_t>& out,
                                    ValidationState& state,
                                    int minLen,
                                    int maxLen) {
  uint8_t lengthLength;
  if (!stream.readBENoExcept<uint8_t>(lengthLength, state)) {
    return state.Invalid("invalid-length-of-length");
  }
  if (!checkRangeNoExcept(lengthLength, minLen, maxLen, state)) {
    return state.Invalid("invalid-length-of-length");
  }
  return stream.readSliceNoExcept(lengthLength, out, state);
}

void writeSingleByteLenValue(WriteStream& stream, Slice<const uint8_t> value) {
  checkRange(value.size(), 0, (std::numeric_limits<uint8_t>::max)());
  stream.writeBE<uint8_t>((uint8_t)value.size());
  stream.write(value);
}

void writeSingleBEValue(WriteStream& stream, int64_t value) {
  std::vector<uint8_t> dataBytes = trimmedArray(value);
  stream.writeBE<uint8_t>((uint8_t)dataBytes.size());
  stream.write(dataBytes);
}

void writeVarLenValue(WriteStream& stream, Slice<const uint8_t> value) {
  writeSingleBEValue(stream, value.size());
  stream.write(value);
}

NetworkBytePair readNetworkByte(ReadStream& stream, TxType type) {
  NetworkBytePair ret;
  auto networkOrType = stream.readBE<uint8_t>();

  if (networkOrType == (uint8_t)type) {
    ret.typeId = networkOrType;
  } else {
    ret.hasNetworkByte = true;
    ret.networkByte = networkOrType;
    ret.typeId = stream.readBE<uint8_t>();
  }

  return ret;
}

void writeNetworkByte(WriteStream& stream, NetworkBytePair networkOrType) {
  if (networkOrType.hasNetworkByte) {
    stream.writeBE<uint8_t>(networkOrType.networkByte);
  }
  stream.writeBE<uint8_t>(networkOrType.typeId);
}

std::string readString(ReadStream& stream) {
  const auto count = readSingleBEValue<int32_t>(stream);

  std::string result;
  result.reserve(count);

  for (int32_t i = 0; i < count; ++i) {
    result.push_back(stream.readBE<char>());
  }

  return result;
}

void writeDouble(WriteStream& stream, const double& val) {
  uint64_t d = 0;
  memcpy(&d, &val, sizeof(val));
  stream.writeBE<uint64_t>(d);
}

double readDouble(ReadStream& stream) {
  uint64_t d = stream.readBE<uint64_t>();
  double r = 0;
  memcpy(&r, &d, sizeof(d));
  return r;
}
}  // namespace altintegration

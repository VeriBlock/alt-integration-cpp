// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>
#include <veriblock/serde.hpp>

namespace altintegration {

bool checkRange(int64_t num, int64_t min, int64_t max, ValidationState& state) {
  if (num < min) {
    return state.Invalid(
        "range-below",
        fmt::format(
            "Expected num to be more or equal to {}, but got {}", min, num));
  }

  if (num > max) {
    return state.Invalid(
        "range-above",
        fmt::format(
            "Expected num to be less or equal than {}, but got {}", max, num));
  }
  return true;
}

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

bool readVarLenValue(ReadStream& stream,
                     Slice<const uint8_t>& out,
                     ValidationState& state,
                     int32_t minLen,
                     int32_t maxLen) {
  int32_t length = 0;
  if (!readSingleBEValue<int32_t>(stream, length, state)) {
    return state.Invalid("readvarlen-bad-length");
  }
  if (!checkRange(length, minLen, maxLen, state)) {
    return state.Invalid("readvarlen-bad-range");
  }
  return stream.readSlice(length, out, state);
}

bool readSingleByteLenValue(ReadStream& stream,
                            Slice<const uint8_t>& out,
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
  return stream.readSlice(length, out, state);
}

void writeSingleByteLenValue(WriteStream& stream, Slice<const uint8_t> value) {
  ValidationState state;
  VBK_ASSERT_MSG(
      checkRange(value.size(), 0, (std::numeric_limits<uint8_t>::max)(), state),
      "Can not writeSingleByteLen: " + state.toString());
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

bool readNetworkByte(ReadStream& stream,
                     TxType type,
                     NetworkBytePair& ret,
                     ValidationState& state) {
  uint8_t networkOrType = 0;
  if (!stream.readBE<uint8_t>(networkOrType, state)) {
    return state.Invalid("bad-network-byte");
  }

  if (networkOrType == (uint8_t)type) {
    ret.typeId = networkOrType;
  } else {
    ret.hasNetworkByte = true;
    ret.networkByte = networkOrType;
    if (!stream.readBE<uint8_t>(ret.typeId, state)) {
      return state.Invalid("bad-type-id");
    }
  }
  return true;
}

void writeNetworkByte(WriteStream& stream, NetworkBytePair networkOrType) {
  if (networkOrType.hasNetworkByte) {
    stream.writeBE<uint8_t>(networkOrType.networkByte);
  }
  stream.writeBE<uint8_t>(networkOrType.typeId);
}

}  // namespace altintegration

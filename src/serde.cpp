#include <veriblock/serde.hpp>

namespace VeriBlock {

void checkRange(int64_t num, int64_t min, int64_t max) {
  if (num < min) {
    throw std::out_of_range("value is less than minimal");
  }

  if (num > max) {
    throw std::out_of_range("value is greater than maximum");
  }
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

Slice<const uint8_t> readVarLenValue(ReadStream& stream,
                                     int minLen,
                                     int maxLen) {
  auto length = readSingleBEValue<int32_t>(stream);
  checkRange(length, minLen, maxLen);
  return stream.readSlice(length);
}

Slice<const uint8_t> readSingleByteLenValue(ReadStream& stream,
                                            int minLen,
                                            int maxLen) {
  const auto lengthLength = stream.readBE<uint8_t>();
  checkRange(lengthLength, minLen, maxLen);
  return stream.readSlice(lengthLength);
}

void writeSingleByteLenValue(WriteStream& stream,
                             const Slice<const uint8_t>& value) {
  checkRange(value.size(), 0, std::numeric_limits<uint8_t>::max());
  stream.writeBE<uint8_t>((uint8_t)value.size());
  stream.write(value);
}

void writeSingleBEValue(WriteStream& stream, int64_t value) {
  std::vector<uint8_t> dataBytes = trimmedArray(value);
  stream.writeBE<uint8_t>((uint8_t)dataBytes.size());
  stream.write(dataBytes);
}

void writeVarLenValue(WriteStream& stream, const Slice<const uint8_t>& value) {
  writeSingleBEValue(stream, value.size());
  stream.write(value);
}

NetworkBytePair readNetworkByte(ReadStream& stream, TxType type) {
  NetworkBytePair ret;
  auto networkOrType = stream.readBE<uint8_t>();

  if (networkOrType == (uint8_t) type) {
    ret.typeId = networkOrType;
  } else {
    ret.hasNetworkByte = true;
    ret.networkByte = networkOrType;
    ret.typeId = stream.readBE<uint8_t>();
  }

  return ret;
}

}  // namespace VeriBlock
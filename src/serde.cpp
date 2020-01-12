#include <veriblock/serde.hpp>

namespace VeriBlock {
Slice<const uint8_t> readVarLenValue(ReadStream& stream,
                                     int minLen,
                                     int maxLen) {
  auto length = readSingleBEValue<int32_t>(stream);
  checkRange(length, minLen, maxLen);
  return stream.readSlice(length);
}

void checkRange(int64_t num, int64_t min, int64_t max) {
  if (num < min) {
    throw std::out_of_range("value is less than minimal");
  }

  if (num > max) {
    throw std::out_of_range("value is greater than maximum");
  }
}

Slice<const uint8_t> readSingleByteLenValue(ReadStream& stream,
                                            int minLen,
                                            int maxLen) {
  const auto lengthLength = stream.readBE<uint8_t>();
  checkRange(lengthLength, minLen, maxLen);
  return stream.readSlice(lengthLength);
}

NetworkBytePair readNetworkByte(ReadStream& stream, TxType type) {
  NetworkBytePair ret;
  auto networkOrType = stream.readBE<uint8_t>();
  if (networkOrType != type) {
    ret.hasNetworkByte = true;
    ret.networkByte = networkOrType;
    ret.typeId = stream.readBE<uint8_t>();
  } else {
    ret.typeId = networkOrType;
  }

  return ret;
}

}  // namespace VeriBlock
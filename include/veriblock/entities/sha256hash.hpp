#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_SHA256HASH_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_SHA256HASH_HPP_

#include <algorithm>
#include <array>
#include <cstdint>

#include "veriblock/read_stream.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/strutil.hpp"

namespace VeriBlock {

struct Sha256Hash {
  using data_type = std::array<uint8_t, 32>;

  Sha256Hash() = default;

  explicit Sha256Hash(Slice<const uint8_t> v) {
    if (v.size() != data.size()) {
      throw std::invalid_argument("Sha256Hash() invalid input length");
    }

    std::copy(v.begin(), v.end(), data.begin());
  }

  static Sha256Hash fromVbk(ReadStream& stream) {
    auto data = readSingleByteLenValue(stream, 32, 32);
    return Sha256Hash(data);
  }

  static Sha256Hash fromRaw(ReadStream& stream) {
    return Sha256Hash(stream.readSlice(32));
  }

  data_type reverse() {
    data_type copy = data;
    std::reverse(copy.begin(), copy.end());
    return copy;
  }

  std::string toHex() const { return HexStr(data); }

  const data_type& getData() const { return data; }

  data_type& getData() { return data; }

 private:
  data_type data{};
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_SHA256HASH_HPP_

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBLAKEHASH_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBLAKEHASH_HPP_

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#include "veriblock/read_stream.hpp"
#include "veriblock/strutil.hpp"

namespace VeriBlock {

struct VBlakeHash {
  explicit VBlakeHash(std::vector<uint8_t> v) : data(std::move(v)) {}

  VBlakeHash trim(size_t len) {
    if (data.size() < len) {
      throw std::out_of_range(
          "VBlakeHash.trim(): can not trim, size of data is less than "
          "requested");
    }

    const auto diff = data.size() - len;
    std::vector<uint8_t> trimmed(data.begin() + diff, data.end());
    return VBlakeHash(std::move(trimmed));
  }

  static VBlakeHash fromRaw(ReadStream& stream, size_t size) {
    return VBlakeHash(stream.read(size));
  }

  std::string toHex() const { return HexStr(data); }

 private:
  std::vector<uint8_t> data;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBLAKEHASH_HPP_

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_COIN_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_COIN_HPP_

#include <cstdint>

#include "veriblock/serde.hpp"

namespace VeriBlock {

struct Coin {
  explicit Coin(int64_t atomicUnits) : units(atomicUnits) {}

  static Coin fromVbkEncoding(ReadStream& stream) {
    return Coin(readSingleBEValue<int64_t>(stream, 0, 8));
  }

  Coin& operator=(int64_t val) {
    this->units = val;
    return *this;
  }

  bool operator==(const Coin& other) const noexcept {
    return units == other.units;
  }

  explicit operator int64_t() const noexcept { return units; }

 private:
  int64_t units;
};

}  // namespace VeriBlock
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_COIN_HPP_

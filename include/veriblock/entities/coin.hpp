#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_COIN_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_COIN_HPP_

#include <cstdint>

#include "veriblock/serde.hpp"

namespace altintegration {

struct Coin {
  int64_t units{};

  Coin() = default;

  explicit Coin(int64_t atomicUnits) : units(atomicUnits) {}

  /**
   * Read VBK data from the stream and convert it to Coin
   * @param stream data stream to read from
   * @return Coin
   */
  static Coin fromVbkEncoding(ReadStream& stream);

  /**
   * Convert Coin to data stream using BtcTx Coin byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Compare two Coins for equality
   * @param other Coin
   * @result comparing result
   */
  bool operator==(const Coin& other) const noexcept;

};

}  // namespace altintegration
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_COIN_HPP_

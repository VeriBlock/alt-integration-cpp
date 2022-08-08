// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_COIN_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_COIN_HPP_

#include <veriblock/pop/fmt.hpp>
#include <veriblock/pop/serde.hpp>
#include <cstddef>
#include <cstdint>
#include <string>

namespace altintegration {
class ValidationState;
class WriteStream;
struct ReadStream;

//! represents VBK atomic units
struct Coin {
  int64_t units{};

  Coin() = default;

  std::string toPrettyString() const;

  explicit Coin(int64_t atomicUnits) : units(atomicUnits) {}

  /**
   * Convert Coin to data stream using BtcTx Coin byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  size_t estimateSize() const;

  /**
   * Compare two Coins for equality
   * @param other Coin
   * @result comparing result
   */
  bool operator==(const Coin& other) const noexcept;
};

//! @overload
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                Coin& out,
                                ValidationState& state);

}  // namespace altintegration
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_COIN_HPP_

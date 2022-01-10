// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCTX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCTX_HPP_

#include <cstdint>
#include <utility>
#include <vector>
#include <veriblock/pop/hashutil.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/slice.hpp>
#include <veriblock/pop/uint.hpp>

namespace altintegration {

/**
 * @struct BtcTx
 *
 * Bitcoin transaction representation.
 *
 */
struct BtcTx {
  using hash_t = uint256;
  std::vector<uint8_t> tx{};

  BtcTx() = default;
  BtcTx(std::vector<uint8_t> bytes) : tx(std::move(bytes)) {}
  BtcTx(Slice<const uint8_t> slice) : tx(slice.begin(), slice.end()) {}

  friend bool operator==(const BtcTx& a, const BtcTx& b) {
    return a.tx == b.tx;
  }

  friend bool operator!=(const BtcTx& a, const BtcTx& b) {
    return !(a.tx == b.tx);
  }

  /**
   * Convert BtcTx to data stream using BtcTx VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert BtcTx to data stream using BtcTx basic byte format
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;

  size_t estimateSize() const;

  /**
   * Calculate the hash of the btc transaction
   * @return hash transaction hash
   */
  hash_t getHash() const;
};

//! @overload
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                BtcTx& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCTX_HPP_

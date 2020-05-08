// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCTX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCTX_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/slice.hpp"
#include "veriblock/uint.hpp"

namespace altintegration {

struct BtcTx {
  using hash_t = uint256;
  std::vector<uint8_t> tx{};

  BtcTx() = default;

  BtcTx(const std::vector<uint8_t>& bytes) : tx(bytes) {}
  BtcTx(Slice<const uint8_t> slice) : tx(slice.begin(), slice.end()) {}

  friend bool operator==(const BtcTx& a, const BtcTx& b) {
    return a.tx == b.tx;
  }

  /**
   * Read VBK data from the stream and convert it to BtcTx
   * @param stream data stream to read from
   * @return BtcTx
   */
  static BtcTx fromVbkEncoding(ReadStream& stream);

  /**
   * Convert BtcTx to data stream using BtcTx VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Calculate the hash of the btc transaction
   * @return hash transaction hash
   */
  hash_t getHash() const;

  std::string toHex() const;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCTX_HPP_

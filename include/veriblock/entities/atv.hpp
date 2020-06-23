// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include "veriblock/consts.hpp"
#include "veriblock/entities/vbk_merkle_path.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbktx.hpp"
#include "veriblock/fmt.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace altintegration {

struct ATV {
  using id_t = uint256;

  VbkTx transaction{};
  VbkMerklePath merklePath{};
  VbkBlock containingBlock{};
  std::vector<VbkBlock> context{};
  //TODO: remove on command group cache implemented
  // flag to mark invalid command groups.
  bool valid{};

  //! (memory only) indicates whether we already did 'checkATV' on this ATV
  mutable bool checked{false};

  std::string toHex() const { return HexStr(toVbkEncoding()); }

  std::string toPrettyString() const {
    return fmt::sprintf("ATV{containingTx=%s, containingBlock=%s}",
                        transaction.getHash().toHex(),
                        containingBlock.getHash().toHex());
  }

  /**
   * Read VBK data from the stream and convert it to ATV
   * @param stream data stream to read from
   * @return ATV
   */
  static ATV fromVbkEncoding(ReadStream& stream);

  /**
   * Read VBK data from the raw byte representation and convert it to ATV
   * @param string data bytes to read from
   * @return ATV
   */
  static ATV fromVbkEncoding(Slice<const uint8_t> bytes);

  /**
   * Convert ATV to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert ATV to raw bytes data using Vbk byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  /**
   * Calculate a ATV id that is the sha256 hash of the ATV rawBytes
   * @return id sha256 hash
   */
  id_t getId() const;

  static std::string name() { return "ATV"; }

  static ATV fromHex(const std::string& h);

  friend bool operator==(const ATV& a, const ATV& b) {
    return a.getId() == b.getId();
  }

  friend bool operator!=(const ATV& a, const ATV& b) { return !(a == b); }
};

template <typename JsonValue>
JsonValue ToJSON(const ATV& atv) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  json::putStringKV(obj, "id", atv.getId().toHex());
  json::putKV(obj, "transaction", ToJSON<JsonValue>(atv.transaction));
  json::putKV(obj, "merklePath", ToJSON<JsonValue>(atv.merklePath));
  json::putKV(obj, "containingBlock", ToJSON<JsonValue>(atv.containingBlock));
  json::putArrayKV(obj, "context", atv.context);
  return obj;
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_

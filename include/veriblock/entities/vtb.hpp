// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VTB_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VTB_HPP_

#include <vector>

#include "veriblock/entities/vbk_merkle_path.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbkpoptx.hpp"
#include "veriblock/fmt.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace altintegration {

struct VTB {
  using id_t = uint256;
  using containing_block_t = VbkBlock;

  VbkPopTx transaction{};
  VbkMerklePath merklePath{};
  containing_block_t containingBlock{};
  std::vector<VbkBlock> context{};

  //! (memory only) indicates whether we already did 'checkPayloads' on this VTB
  mutable bool checked{false};

  std::string toHex() const { return HexStr(toVbkEncoding()); }

  std::string toPrettyString() const {
    return fmt::sprintf("VTB{containingTx=%s(%s), containingBlock=%s, context=%d blocks starting at %s}",
                        transaction.getHash().toHex(),
                        transaction.toPrettyString(),
                        containingBlock.getHash().toHex(),
                        context.size(),
                        context.size() > 0 ? context[0].toPrettyString() : "(none)"
                       );
  }

  /**
   * Read VBK data from the stream and convert it to VTB
   * @param stream data stream to read from
   * @return VTB
   */
  static VTB fromVbkEncoding(ReadStream& stream);

  /**
   * Read VBK data from the raw byte representation and convert it to VTB
   * @param string data bytes to read from
   * @return VTB
   */
  static VTB fromVbkEncoding(Slice<const uint8_t> bytes);

  /**
   * Read VBK data from the string raw byte representation and convert it to VTB
   * @param string data bytes to read from
   * @return VTB
   */
  static VTB fromVbkEncoding(const std::string& bytes);

  /**
   * Convert VTB to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  static VTB fromHex(const std::string& hex);

  /**
   * Convert VTB to raw bytes data using Vbk byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  /**
   * Calculate a VTB id that is the sha256 hash of the VTB rawBytes
   * @return id sha256 hash
   */
  id_t getId() const;

  static std::string name() { return "VTB"; }

  friend bool operator==(const VTB& a, const VTB& b) {
    return a.getId() == b.getId();
  }

  friend bool operator!=(const VTB& a, const VTB& b) { return !(a == b); }
};

template <typename JsonValue>
JsonValue ToJSON(const VTB& v) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  json::putStringKV(obj, "id", v.getId().toHex());
  json::putKV(obj, "transaction", ToJSON<JsonValue>(v.transaction));
  json::putKV(obj, "merklePath", ToJSON<JsonValue>(v.merklePath));
  json::putKV(obj, "containingBlock", ToJSON<JsonValue>(v.containingBlock));
  json::putArrayKV(obj, "context", v.context);
  return obj;
}

inline void PrintTo(const VTB& v, std::ostream* os) {
  *os << ToJSON<picojson::value>(v).serialize(false);
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VTB_HPP_

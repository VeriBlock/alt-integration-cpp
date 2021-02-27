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
#include "veriblock/json.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace altintegration {

/**
 * @struct VTB
 *
 * Veriblock to Bitcoin publication, committed to Veriblock blockchain in
 * containingBlock.
 *
 * @ingroup entities
 */
struct VTB {
  using id_t = uint256;
  using containing_block_t = VbkBlock;

  uint32_t version = 1;
  VbkPopTx transaction{};
  VbkMerklePath merklePath{};
  containing_block_t containingBlock{};

  //! (memory only) indicates whether we already did 'checkPayloads' on this VTB
  mutable bool checked{false};

  std::string toPrettyString() const;

  /**
   * Convert VTB to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert VTB to raw bytes data using Vbk byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  size_t estimateSize() const;

  /**
   * Calculate a VTB id that is the sha256 hash of the VTB rawBytes
   * @return id sha256 hash
   */
  id_t getId() const;

  static const std::string& name() { return _name; }

  friend bool operator==(const VTB& a, const VTB& b) {
    return a.getId() == b.getId();
  }

  friend bool operator!=(const VTB& a, const VTB& b) { return !(a == b); }

  friend bool operator<(const VTB& a, const VTB& b) {
    return a.containingBlock.getHeight() < b.containingBlock.getHeight();
  }

 private:
  static const std::string _name;
};

//! @private
template <typename JsonValue>
JsonValue ToJSON(const VTB& v) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  json::putStringKV(obj, "id", v.getId().toHex());
  json::putIntKV(obj, "version", v.version);
  json::putKV(obj, "transaction", ToJSON<JsonValue>(v.transaction));
  json::putKV(obj, "merklePath", ToJSON<JsonValue>(v.merklePath));
  json::putKV(obj, "containingBlock", ToJSON<JsonValue>(v.containingBlock));
  return obj;
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                VTB& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VTB_HPP_

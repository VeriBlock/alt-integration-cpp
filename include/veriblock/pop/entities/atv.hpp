// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_

#include <cstdint>
#include <string>
#include <vector>
#include <veriblock/pop/consts.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/uint.hpp>

#include "vbk_merkle_path.hpp"
#include "vbkblock.hpp"
#include "vbktx.hpp"

namespace altintegration {

/**
 * @class ATV
 */
struct ATV {
  using id_t = uint256;

  uint32_t version = 1;
  VbkTx transaction{};
  VbkMerklePath merklePath{};
  VbkBlock blockOfProof{};

  //! (memory only) indicates whether we already did 'checkATV' on this ATV
  mutable bool checked{false};

  std::string toPrettyString() const;

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

  size_t estimateSize() const;

  /**
   * Calculate a ATV id that is the sha256 hash of the ATV rawBytes
   * @return id sha256 hash
   */
  id_t getId() const;

  static const std::string& name() { return _name; }

  friend bool operator==(const ATV& a, const ATV& b) {
    return a.getId() == b.getId();
  }

  friend bool operator!=(const ATV& a, const ATV& b) { return !(a == b); }

  friend bool operator<(const ATV& a, const ATV& b) {
    return a.blockOfProof.getHeight() < b.blockOfProof.getHeight();
  }

 private:
  static const std::string _name;
};

template <typename JsonValue>
JsonValue ToJSON(const ATV& atv) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  json::putStringKV(obj, "id", atv.getId().toHex());
  json::putIntKV(obj, "version", atv.version);
  json::putKV(obj, "transaction", ToJSON<JsonValue>(atv.transaction));
  json::putKV(obj, "merklePath", ToJSON<JsonValue>(atv.merklePath));
  json::putKV(obj, "blockOfProof", ToJSON<JsonValue>(atv.blockOfProof));
  return obj;
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                ATV& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_

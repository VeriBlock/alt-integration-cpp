// Copyright (c) 2019-2021 Xenios SEZC
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
#include <veriblock/pop/type_traits.hpp>
#include <veriblock/pop/uint.hpp>

#include "vbk_merkle_path.hpp"
#include "vbkblock.hpp"
#include "vbktx.hpp"

namespace altintegration {

/**
 * @class ATV
 *
 * Atlchain endorsement.
 */
struct ATV {
  using id_t = uint256;

  //! ATV serialization version.
  uint32_t version = 1;

  //! endorsing transaction.
  VbkTx transaction{};

  //! merkle path that proves that endorsing transaction is in blockOfProof.
  VbkMerklePath merklePath{};

  //! VBK block which contains endorsing transaction.
  VbkBlock blockOfProof{};

  //! (memory only) indicates whether we already did 'checkATV' on this ATV
  mutable bool checked{false};

  //! create pretty string
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

  //! Estimate serialization size.
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

//! @private
template <>
struct IsPopPayload<ATV> {
  static const bool value = true;
};

//! @overload
template <typename JsonValue>
JsonValue ToJSON(const ATV& atv) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  json::putIntKV(obj, "version", atv.version);
  json::putKV(obj, "transaction", ToJSON<JsonValue>(atv.transaction));
  json::putKV(obj, "merklePath", ToJSON<JsonValue>(atv.merklePath));
  json::putKV(obj, "blockOfProof", ToJSON<JsonValue>(atv.blockOfProof));

  // return this entity in VBK-serialized form for easy consumption.
  // DO NOT REMOVE these fields - otherwise Stratum compat will break.
  json::putStringKV(obj, "id", atv.getId().toHex());
  json::putStringKV(obj, "serialized", SerializeToHex(atv));
  return obj;
}

//! @overload
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                ATV& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_

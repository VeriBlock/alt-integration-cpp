// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_

#include <veriblock/pop/consts.hpp>
#include <veriblock/pop/hashutil.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/slice.hpp>
#include <veriblock/pop/uint.hpp>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "address.hpp"
#include "coin.hpp"
#include "output.hpp"
#include "publication_data.hpp"
#include "veriblock/pop/blob.hpp"
#include "veriblock/pop/entities/network_byte_pair.hpp"
#include "veriblock/pop/json.hpp"
#include "veriblock/pop/strutil.hpp"

namespace altintegration {
class ValidationState;
class WriteStream;
struct ReadStream;
template <class ElementType> struct Slice;

/**
 * @struct VbkTx
 *
 * Veriblock transaction, which endorses ALT block in VBK blockchain.
 *
 */
struct VbkTx {
  using hash_t = uint256;

  NetworkBytePair networkOrType{};
  Address sourceAddress{};
  Coin sourceAmount{};
  std::vector<Output> outputs{};
  int64_t signatureIndex{};
  PublicationData publicationData{};
  std::vector<uint8_t> signature{};
  std::vector<uint8_t> publicKey{};

  /**
   * Convert VbkTx to data stream using VbkTx VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert VbkTx to raw bytes data using VBK byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  size_t estimateSize() const;

  /**
   * Calculate the hash of the VBK transaction
   * @return hash VBK transaction hash
   */
  uint256 getHash() const;

  /**
   * Calculate the fee of the VBK transaction
   * @return amount spent as tx fee
   */
  Coin calculateTxFee() const;

  friend bool operator==(const VbkTx& a, const VbkTx& b) {
    return a.getHash() == b.getHash();
  }

  friend bool operator!=(const VbkTx& a, const VbkTx& b) { return !(a == b); }

 private:
  /**
   * Convert VbkTx to data stream using VbkTx basic byte format without
   * signature and publicKey data
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;
};

//! @overload
template <typename JsonValue>
JsonValue ToJSON(const VbkTx& tx) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  tx.networkOrType.networkType.template putJson<JsonValue>(obj);
  json::putStringKV(obj, "hash", tx.getHash().toHex());
  json::putIntKV(obj, "type", tx.networkOrType.typeId);
  json::putStringKV(obj, "sourceAddress", tx.sourceAddress.toString());
  json::putIntKV(obj, "sourceAmount", tx.sourceAmount.units);
  json::putArrayKV(obj, "outputs", tx.outputs);
  json::putIntKV(obj, "signatureIndex", tx.signatureIndex);
  json::putKV(obj, "publicationData", ToJSON<JsonValue>(tx.publicationData));
  json::putStringKV(obj, "signature", HexStr(tx.signature));
  json::putStringKV(obj, "publicKey", HexStr(tx.publicKey));
  return obj;
}

//! @overload
bool DeserializeFromRaw(ReadStream& stream,
                        Slice<const uint8_t> signature,
                        Slice<const uint8_t> publicKey,
                        VbkTx& out,
                        ValidationState& state);

//! @overload
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                VbkTx& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_

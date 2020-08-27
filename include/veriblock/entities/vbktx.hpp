// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/consts.hpp"
#include "veriblock/entities/address.hpp"
#include "veriblock/entities/coin.hpp"
#include "veriblock/entities/output.hpp"
#include "veriblock/entities/publication_data.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/slice.hpp"
#include "veriblock/uint.hpp"

namespace altintegration {

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
   * Read basic data from the stream, signature, publicKey and convert it to
   * VbkTx
   * @param stream data stream to read from
   * @param _signature bytes
   * @param _publicKey bytes
   * @return VbkTx
   */
  static VbkTx fromRaw(ReadStream& stream,
                       Slice<const uint8_t> _signature,
                       Slice<const uint8_t> _publicKey);

  /**
   * Read VBK data from the stream and convert it to VbkTx
   * @param stream data stream to read from
   * @return VbkTx
   */
  static VbkTx fromVbkEncoding(ReadStream& stream);

  /**
   * Convert VbkTx to data stream using VbkTx basic byte format without
   * signature and publicKey data
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;

  /**
   * Convert VbkTx to data stream using VbkTx VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Calculate the hash of the vbk transaction
   * @return hash vbk transaction hash
   */
  uint256 getHash() const;
};

template <typename JsonValue>
JsonValue ToJSON(const VbkTx& tx) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  if (tx.networkOrType.hasNetworkByte) {
    json::putIntKV(obj, "networkByte", tx.networkOrType.networkByte);
  } else {
    json::putNullKV(obj, "networkByte");
  }
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

bool DeserializeRaw(ReadStream& stream,
                    Slice<const uint8_t> signature,
                    Slice<const uint8_t> publicKey,
                    VbkTx& out,
                    ValidationState& state);

bool DeserializeRaw(Slice<const uint8_t> data,
                    Slice<const uint8_t> signature,
                    Slice<const uint8_t> publicKey,
                    VbkTx& out,
                    ValidationState& state);

bool Deserialize(ReadStream& stream, VbkTx& out, ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_

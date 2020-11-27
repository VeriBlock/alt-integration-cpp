// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/consts.hpp"
#include "veriblock/entities/address.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/btctx.hpp"
#include "veriblock/entities/merkle_path.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/slice.hpp"

namespace altintegration {

/**
 * @struct VbkPopTx
 *
 * Veriblock POP transaction, which endorses VBK block in BTC blockchain.
 *
 * @ingroup entities
 */
struct VbkPopTx {
  using hash_t = uint256;

  NetworkBytePair networkOrType{};
  Address address{};
  VbkBlock publishedBlock{};
  BtcTx bitcoinTransaction{};
  MerklePath merklePath{};
  BtcBlock blockOfProof{};
  std::vector<BtcBlock> blockOfProofContext{};
  std::vector<uint8_t> signature{};
  std::vector<uint8_t> publicKey{};

  /**
   * Convert VbkPopTx to data stream using VbkPopTx basic byte format without
   * signature and publicKey data
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;

  /**
   * Convert VbkPopTx to data stream using VbkPopTx VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Calculate the hash of the vbk pop transaction
   * @return hash vbk pop transaction hash
   */
  hash_t getHash() const;

  std::string toPrettyString() const;
};

template <typename JsonValue>
JsonValue ToJSON(const VbkPopTx& tx) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  if (tx.networkOrType.hasNetworkByte) {
    json::putIntKV(obj, "networkByte", tx.networkOrType.networkByte);
  } else {
    json::putNullKV(obj, "networkByte");
  }
  json::putStringKV(obj, "hash", tx.getHash().toHex());
  json::putIntKV(obj, "type", tx.networkOrType.typeId);
  json::putStringKV(obj, "address", tx.address.toString());
  json::putKV(obj, "publishedBlock", ToJSON<JsonValue>(tx.publishedBlock));
  json::putStringKV(
      obj, "bitcoinTransaction", SerializeToRawHex(tx.bitcoinTransaction));
  json::putKV(obj, "merklePath", ToJSON<JsonValue>(tx.merklePath));
  json::putKV(obj, "blockOfProof", ToJSON<JsonValue>(tx.blockOfProof));
  json::putArrayKV(obj, "blockOfProofContext", tx.blockOfProofContext);
  json::putStringKV(obj, "signature", HexStr(tx.signature));
  json::putStringKV(obj, "publicKey", HexStr(tx.publicKey));
  return obj;
}

bool DeserializeFromRaw(ReadStream& stream,
                        Slice<const uint8_t> signature,
                        Slice<const uint8_t> publicKey,
                        VbkPopTx& out,
                        ValidationState& state);

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                VbkPopTx& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_

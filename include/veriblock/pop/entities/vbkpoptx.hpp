// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_

#include <veriblock/pop/consts.hpp>
#include <veriblock/pop/hashutil.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/slice.hpp>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

#include "address.hpp"
#include "btcblock.hpp"
#include "btctx.hpp"
#include "merkle_path.hpp"
#include "vbkblock.hpp"
#include "veriblock/pop/blob.hpp"
#include "veriblock/pop/entities/network_byte_pair.hpp"
#include "veriblock/pop/json.hpp"
#include "veriblock/pop/strutil.hpp"
#include "veriblock/pop/uint.hpp"

namespace altintegration {
class ValidationState;
class WriteStream;
struct ReadStream;
template <class ElementType> struct Slice;

/**
 * @struct VbkPopTx
 *
 * Veriblock POP transaction, which endorses VBK block in BTC blockchain.
 *
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
   * Convert VbkPopTx to data stream using VbkPopTx VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  size_t estimateSize() const;

  /**
   * Calculate the hash of the vbk pop transaction
   * @return hash vbk pop transaction hash
   */
  hash_t getHash() const;

  std::string toPrettyString() const;

  friend bool operator==(const VbkPopTx& a, const VbkPopTx& b) {
    return a.getHash() == b.getHash();
  }

  friend bool operator!=(const VbkPopTx& a, const VbkPopTx& b) {
    return !(a == b);
  }

 private:
  /**
   * Convert VbkPopTx to data stream using VbkPopTx basic byte format without
   * signature and publicKey data
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;
};

//! @overload
template <typename JsonValue>
JsonValue ToJSON(const VbkPopTx& tx) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  tx.networkOrType.networkType.template putJson<JsonValue>(obj);
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

//! @overload
bool DeserializeFromRaw(ReadStream& stream,
                        Slice<const uint8_t> signature,
                        Slice<const uint8_t> publicKey,
                        VbkPopTx& out,
                        ValidationState& state);

//! @overload
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                VbkPopTx& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_

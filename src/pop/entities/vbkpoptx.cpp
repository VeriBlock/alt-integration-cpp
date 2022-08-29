// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <algorithm>
#include <memory>
#include <veriblock/pop/entities/vbkpoptx.hpp>

#include "veriblock/pop/consts.hpp"
#include "veriblock/pop/entities/address.hpp"
#include "veriblock/pop/entities/btcblock.hpp"
#include "veriblock/pop/entities/btctx.hpp"
#include "veriblock/pop/entities/merkle_path.hpp"
#include "veriblock/pop/entities/vbkblock.hpp"
#include "veriblock/pop/hashutil.hpp"
#include "veriblock/pop/logger.hpp"
#include "veriblock/pop/read_stream.hpp"
#include "veriblock/pop/serde.hpp"
#include "veriblock/pop/slice.hpp"
#include "veriblock/pop/validation_state.hpp"
#include "veriblock/pop/write_stream.hpp"

namespace altintegration {

void VbkPopTx::toRaw(WriteStream& stream) const {
  writeNetworkByte(stream, networkOrType);
  address.toVbkEncoding(stream);
  publishedBlock.toVbkEncoding(stream);
  bitcoinTransaction.toVbkEncoding(stream);
  merklePath.toVbkEncoding(stream);
  blockOfProof.toVbkEncoding(stream);

  writeSingleBEValue(stream, blockOfProofContext.size());
  for (const auto& block : blockOfProofContext) {
    block.toVbkEncoding(stream);
  }
}

void VbkPopTx::toVbkEncoding(WriteStream& stream) const {
  WriteStream txStream;
  toRaw(txStream);
  writeVarLenValue(stream, txStream.data());
  writeSingleByteLenValue(stream, signature);
  writeSingleByteLenValue(stream, publicKey);
}

size_t VbkPopTx::estimateSize() const {
  size_t rawSize = 0;
  rawSize += networkByteSize(networkOrType);
  rawSize += address.estimateSize();
  rawSize += publishedBlock.estimateSize();
  rawSize += bitcoinTransaction.estimateSize();
  rawSize += merklePath.estimateSize();
  rawSize += blockOfProof.estimateSize();

  rawSize += singleBEValueSize(blockOfProofContext.size());
  for (const auto& block : blockOfProofContext) {
    rawSize += block.estimateSize();
  }

  size_t size = 0;
  size += varLenValueSize(rawSize);
  size += singleByteLenValueSize(signature);
  size += singleByteLenValueSize(publicKey);

  return size;
}

uint256 VbkPopTx::getHash() const {
  WriteStream stream;
  toRaw(stream);
  return sha256(stream.data());
}

std::string VbkPopTx::toPrettyString() const {
  return format(
      "VbkPopTx{{address={}, publishedBlock={}, blockOfProof={}, context={} "
      "blocks starting with {}}}",
      address.toString(),
      publishedBlock.toPrettyString(),
      blockOfProof.toPrettyString(),
      blockOfProofContext.size(),
      blockOfProofContext.size() > 0 ? blockOfProofContext[0].toPrettyString()
                                     : "(none)");
}

bool DeserializeFromRaw(ReadStream& stream,
                        Slice<const uint8_t> signature,
                        Slice<const uint8_t> publicKey,
                        VbkPopTx& tx,
                        ValidationState& state) {
  if (!readNetworkByte(stream, TxType::VBK_POP_TX, tx.networkOrType, state)) {
    return state.Invalid("vbkpoptx-network-or-type");
  }

  if (!DeserializeFromVbkEncoding(stream, tx.address, state)) {
    return state.Invalid("vbkpoptx-address");
  }
  if (!DeserializeFromVbkEncoding(stream, tx.publishedBlock, state)) {
    return state.Invalid("vbkpoptx-published-block");
  }
  if (!DeserializeFromVbkEncoding(stream, tx.bitcoinTransaction, state)) {
    return state.Invalid("vbkpoptx-bitcoin-tx");
  }

  auto hash = sha256twice(tx.bitcoinTransaction.tx);
  if (!DeserializeFromVbkEncoding(stream, hash, tx.merklePath, state)) {
    return state.Invalid("vbkpoptx-merkle-path");
  }

  if (!DeserializeFromVbkEncoding(stream, tx.blockOfProof, state)) {
    return state.Invalid("vbkpoptx-block-of-proof");
  }

  if (!readArrayOf<BtcBlock>(
          stream,
          tx.blockOfProofContext,
          state,
          0,
          MAX_BTC_BLOCKS_IN_VBKPOPTX,
          [](ReadStream& stream, BtcBlock& out, ValidationState& state) {
            return DeserializeFromVbkEncoding(stream, out, state);
          })) {
    return state.Invalid("vbkpoptx-btc-context");
  }

  tx.signature = signature.asVector();
  tx.publicKey = publicKey.asVector();
  return true;
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                VbkPopTx& out,
                                ValidationState& state) {
  Slice<const uint8_t> rawTx;
  if (!readVarLenValue(stream, rawTx, state, 0, MAX_POPDATA_SIZE)) {
    return state.Invalid("vbkpoptx-invalid-tx");
  }
  Slice<const uint8_t> signature;
  if (!readSingleByteLenValue(
          stream, signature, state, 0, MAX_SIGNATURE_SIZE)) {
    return state.Invalid("vbkpoptx-invalid-signature");
  }
  Slice<const uint8_t> publicKey;
  if (!readSingleByteLenValue(
          stream, publicKey, state, 0, MAX_PUBLIC_KEY_SIZE)) {
    return state.Invalid("vbkpoptx-invalid-public-key");
  }

  ReadStream txstream(rawTx);
  return DeserializeFromRaw(txstream, signature, publicKey, out, state);
}

}  // namespace altintegration

// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/vbkpoptx.hpp"

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

uint256 VbkPopTx::getHash() const {
  WriteStream stream;
  toRaw(stream);
  return sha256(stream.data());
}

std::string VbkPopTx::toPrettyString() const {
  return fmt::sprintf(
      "VbkPopTx{address=%s, publishedBlock=%s, blockOfProof=%s, context= %d "
      "blocks starting with %s}",
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

  if (!readArrayOf<BtcBlock>(stream,
                             tx.blockOfProofContext,
                             state,
                             0,
                             MAX_CONTEXT_COUNT_VBK_PUBLICATION,
                             [&](BtcBlock& out) {
                               return DeserializeFromVbkEncoding(
                                   stream, out, state);
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
  if (!readVarLenValue(stream, rawTx, state, 0, MAX_RAWTX_SIZE_VBKPOPTX)) {
    return state.Invalid("vbkpoptx-invalid-tx");
  }
  Slice<const uint8_t> signature;
  if (!readSingleByteLenValue(
          stream, signature, state, 0, MAX_SIGNATURE_SIZE)) {
    return state.Invalid("vbkpoptx-invalid-signature");
  }
  Slice<const uint8_t> publicKey;
  if (!readSingleByteLenValue(stream, publicKey, state, 0, PUBLIC_KEY_SIZE)) {
    return state.Invalid("vbkpoptx-invalid-public-key");
  }

  ReadStream txstream(rawTx);
  return DeserializeFromRaw(txstream, signature, publicKey, out, state);
}

}  // namespace altintegration

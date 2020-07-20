// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/vbkpoptx.hpp"

namespace altintegration {

VbkPopTx VbkPopTx::fromRaw(ReadStream& stream,
                           Slice<const uint8_t> _signature,
                           Slice<const uint8_t> _publicKey) {
  VbkPopTx tx{};
  tx.networkOrType = readNetworkByte(stream, TxType::VBK_POP_TX);
  tx.address = Address::fromVbkEncoding(stream);
  tx.publishedBlock = VbkBlock::fromVbkEncoding(stream);
  tx.bitcoinTransaction = BtcTx::fromVbkEncoding(stream);

  auto hash = sha256twice(tx.bitcoinTransaction.tx);
  tx.merklePath = MerklePath::fromVbkEncoding(stream, hash);
  tx.blockOfProof = BtcBlock::fromVbkEncoding(stream);

  tx.blockOfProofContext = readArrayOf<BtcBlock>(
      stream, 0, MAX_CONTEXT_COUNT, BtcBlock::fromVbkEncoding);
  tx.signature = std::vector<uint8_t>(_signature.begin(), _signature.end());
  tx.publicKey = std::vector<uint8_t>(_publicKey.begin(), _publicKey.end());

  return tx;
}

VbkPopTx VbkPopTx::fromVbkEncoding(ReadStream& stream) {
  auto rawTx = readVarLenValue(stream, 0, MAX_RAWTX_SIZE_VBKPOPTX);
  auto signature = readSingleByteLenValue(stream, 0, MAX_SIGNATURE_SIZE);
  auto publicKey = readSingleByteLenValue(stream, 0, PUBLIC_KEY_SIZE);
  ReadStream rawTxStream(rawTx);
  return fromRaw(rawTxStream, signature, publicKey);
}

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
      "VbkPopTx{address=%s, publishedBlock=%s, blockOfProof=%s, context= %d blocks starting with %s}",
      address.toString(),
      publishedBlock.toPrettyString(),
      blockOfProof.toPrettyString(),
      blockOfProofContext.size(),
      blockOfProofContext.size() > 0 ?blockOfProofContext[0].toPrettyString() : "(none)"
                     );
}

}  // namespace altintegration

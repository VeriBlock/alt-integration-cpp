// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/vbktx.hpp"

using namespace altintegration;

void VbkTx::toRaw(WriteStream& stream) const {
  writeNetworkByte(stream, networkOrType);
  sourceAddress.toVbkEncoding(stream);
  sourceAmount.toVbkEncoding(stream);
  stream.writeBE<uint8_t>((uint8_t)outputs.size());
  for (const auto& output : outputs) {
    output.toVbkEncoding(stream);
  }
  writeSingleBEValue(stream, signatureIndex);

  WriteStream pubBytesStream;
  publicationData.toRaw(pubBytesStream);
  writeVarLenValue(stream, pubBytesStream.data());
}

void VbkTx::toVbkEncoding(WriteStream& stream) const {
  WriteStream txStream;
  toRaw(txStream);
  writeVarLenValue(stream, txStream.data());
  writeSingleByteLenValue(stream, signature);
  writeSingleByteLenValue(stream, publicKey);
}

uint256 VbkTx::getHash() const {
  WriteStream stream;
  toRaw(stream);
  return sha256(stream.data());
}

bool altintegration::DeserializeFromRaw(ReadStream& stream,
                                        Slice<const uint8_t> signature,
                                        Slice<const uint8_t> publicKey,
                                        VbkTx& tx,
                                        ValidationState& state) {
  if (!readNetworkByte(stream, TxType::VBK_TX, tx.networkOrType, state)) {
    return state.Invalid("vbktx-network-or-type");
  }

  if (!DeserializeFromVbkEncoding(stream, tx.sourceAddress, state)) {
    return state.Invalid("vbktx-address");
  }
  if (!DeserializeFromVbkEncoding(stream, tx.sourceAmount, state)) {
    return state.Invalid("vbktx-amount");
  }

  uint8_t outputSize = 0;
  if (!stream.readBE<uint8_t>(outputSize, state)) {
    return state.Invalid("vbktx-outputs-size");
  }

  tx.outputs.reserve(outputSize);
  for (size_t i = 0; i < outputSize; i++) {
    Output output;
    if (!DeserializeFromVbkEncoding(stream, output, state)) {
      return state.Invalid("vbktx-output", i);
    }
    tx.outputs.emplace_back(output);
  }

  if (!readSingleBEValue<int64_t>(stream, tx.signatureIndex, state)) {
    return state.Invalid("vbktx-signature-index");
  }

  Slice<const uint8_t> pubBytes;
  if (!readVarLenValue(stream, pubBytes, state, 0, MAX_SIZE_PUBLICATION_DATA)) {
    return state.Invalid("vbktx-publication-bytes");
  }

  if (!DeserializeFromVbkEncoding(pubBytes, tx.publicationData, state)) {
    return state.Invalid("vbktx-publication-data");
  }
  tx.signature = signature.asVector();
  tx.publicKey = publicKey.asVector();

  return true;
}

bool altintegration::DeserializeFromVbkEncoding(ReadStream& stream,
                                                VbkTx& out,
                                                ValidationState& state) {
  Slice<const uint8_t> rawTx;
  if (!readVarLenValue(stream, rawTx, state, 0, MAX_RAWTX_SIZE_VBKTX)) {
    return state.Invalid("vbktx-header");
  }
  Slice<const uint8_t> signature;
  if (!readSingleByteLenValue(
          stream, signature, state, 0, MAX_SIGNATURE_SIZE)) {
    return state.Invalid("vbktx-signature");
  }
  Slice<const uint8_t> publicKey;
  if (!readSingleByteLenValue(stream, publicKey, state, 0, PUBLIC_KEY_SIZE)) {
    return state.Invalid("vbktx-public-key");
  }
  ReadStream txstream(rawTx);
  return DeserializeFromRaw(txstream, signature, publicKey, out, state);
}

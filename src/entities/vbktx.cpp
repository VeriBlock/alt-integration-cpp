// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/vbktx.hpp"

using namespace altintegration;

VbkTx VbkTx::fromRaw(ReadStream& stream,
                     Slice<const uint8_t> _signature,
                     Slice<const uint8_t> _publicKey) {
  VbkTx tx{};
  tx.networkOrType = readNetworkByte(stream, TxType::VBK_TX);
  tx.sourceAddress = Address::fromVbkEncoding(stream);
  tx.sourceAmount = Coin::fromVbkEncoding(stream);

  uint8_t outputSize = stream.readBE<uint8_t>();
  tx.outputs.reserve(outputSize);
  for (size_t i = 0; i < outputSize; i++) {
    tx.outputs.emplace_back(Output::fromVbkEncoding(stream));
  }

  tx.signatureIndex = readSingleBEValue<int64_t>(stream);
  auto pubBytes = readVarLenValue(stream, 0, MAX_SIZE_PUBLICATION_DATA);

  ReadStream pubBytesStream(pubBytes);
  tx.publicationData = PublicationData::fromRaw(pubBytesStream);
  tx.signature = std::vector<uint8_t>(_signature.begin(), _signature.end());
  tx.publicKey = std::vector<uint8_t>(_publicKey.begin(), _publicKey.end());

  return tx;
}

VbkTx VbkTx::fromVbkEncoding(ReadStream& stream) {
  auto rawTx = readVarLenValue(stream, 0, MAX_RAWTX_SIZE_VBKTX);
  auto signature = readSingleByteLenValue(stream, 0, MAX_SIGNATURE_SIZE);
  auto publicKey = readSingleByteLenValue(stream, 0, PUBLIC_KEY_SIZE);
  ReadStream rawTxStream(rawTx);
  return fromRaw(rawTxStream, signature, publicKey);
}

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

bool altintegration::DeserializeRaw(ReadStream& stream,
                                    Slice<const uint8_t> signature,
                                    Slice<const uint8_t> publicKey,
                                    VbkTx& out,
                                    ValidationState& state) {
  VbkTx tx{};

  if (!readNetworkByte(stream, TxType::VBK_TX, tx.networkOrType, state)) {
    return state.Invalid("network-or-type");
  }

  if (!Deserialize(stream, tx.sourceAddress, state)) {
    return state.Invalid("address");
  }
  if (!Deserialize(stream, tx.sourceAmount, state)) {
    return state.Invalid("amount");
  }

  uint8_t outputSize;
  if (!stream.readBE<uint8_t>(outputSize, state)) {
    return state.Invalid("outputs-size");
  }

  tx.outputs.reserve(outputSize);
  for (size_t i = 0; i < outputSize; i++) {
    Output output;
    if (!Deserialize(stream, output, state)) {
      return state.Invalid("output", i);
    }
    tx.outputs.emplace_back(output);
  }

  if (!readSingleBEValue<int64_t>(stream, tx.signatureIndex, state)) {
    return state.Invalid("signature-index");
  }

  Slice<const uint8_t> pubBytes;
  if (!readVarLenValue(stream, pubBytes, state, 0, MAX_SIZE_PUBLICATION_DATA)) {
    return state.Invalid("publication-bytes");
  }

  ReadStream pubBytesStream(pubBytes);
  if (!Deserialize(pubBytes, tx.publicationData, state)) {
    return state.Invalid("publication-data");
  }
  tx.signature = std::vector<uint8_t>(signature.begin(), signature.end());
  tx.publicKey = std::vector<uint8_t>(publicKey.begin(), publicKey.end());

  out = tx;
  return true;
}

bool altintegration::DeserializeRaw(Slice<const uint8_t> data,
                                    Slice<const uint8_t> signature,
                                    Slice<const uint8_t> publicKey,
                                    VbkTx& out,
                                    ValidationState& state) {
  ReadStream stream(data);
  return DeserializeRaw(stream, signature, publicKey, out, state);
}

bool altintegration::Deserialize(ReadStream& stream,
                                 VbkTx& out,
                                 ValidationState& state) {
  Slice<const uint8_t> rawTx;
  if (!readVarLenValue(stream, rawTx, state, 0, MAX_RAWTX_SIZE_VBKTX)) {
    return state.Invalid("bad-header");
  }
  Slice<const uint8_t> signature;
  if (!readSingleByteLenValue(
          stream, signature, state, 0, MAX_SIGNATURE_SIZE)) {
    return state.Invalid("bad-signature");
  }
  Slice<const uint8_t> publicKey;
  if (!readSingleByteLenValue(stream, publicKey, state, 0, PUBLIC_KEY_SIZE)) {
    return state.Invalid("bad-public-key");
  }
  return DeserializeRaw(rawTx, signature, publicKey, out, state);
}

bool altintegration::Deserialize(Slice<const uint8_t> data,
                                 VbkTx& out,
                                 ValidationState& state) {
  ReadStream stream(data);
  return Deserialize(stream, out, state);
}

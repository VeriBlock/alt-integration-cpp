#include "veriblock/entities/vbktx.hpp"

using namespace AltIntegrationLib;

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

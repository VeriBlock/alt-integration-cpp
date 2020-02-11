#include "veriblock/entities/btcblock.hpp"

using namespace VeriBlock;

BtcBlock BtcBlock::fromRaw(const std::vector<uint8_t>& bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

BtcBlock BtcBlock::fromRaw(ReadStream& stream) {
  BtcBlock block{};
  block.version = stream.readLE<uint32_t>();
  block.previousBlock = stream.readSlice(SHA256_HASH_SIZE).reverse();
  block.merkleRoot = stream.readSlice(SHA256_HASH_SIZE).reverse();
  block.timestamp = stream.readLE<uint32_t>();
  block.bits = stream.readLE<uint32_t>();
  block.nonce = stream.readLE<uint32_t>();
  return block;
}

BtcBlock BtcBlock::fromVbkEncoding(ReadStream& stream) {
  ReadStream valStream(
      readSingleByteLenValue(stream, BTC_HEADER_SIZE, BTC_HEADER_SIZE));
  return BtcBlock::fromRaw(valStream);
}

void BtcBlock::toRaw(WriteStream& stream) const {
  stream.writeLE<uint32_t>(version);
  stream.write(previousBlock.reverse());
  stream.write(merkleRoot.reverse());
  stream.writeLE<uint32_t>(timestamp);
  stream.writeLE<uint32_t>(bits);
  stream.writeLE<uint32_t>(nonce);
}

void BtcBlock::toVbkEncoding(WriteStream& stream) const {
  WriteStream blockStream;
  toRaw(blockStream);
  writeSingleByteLenValue(stream, blockStream.data());
}

uint256 BtcBlock::getHash() const {
  WriteStream stream;
  toRaw(stream);
  return sha256twice(stream.data()).reverse();
}

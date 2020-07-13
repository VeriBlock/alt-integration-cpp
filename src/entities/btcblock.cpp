// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/btcblock.hpp"

using namespace altintegration;

BtcBlock BtcBlock::fromRaw(const std::vector<uint8_t>& bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

BtcBlock BtcBlock::fromRaw(ReadStream& stream) {
  BtcBlock block{};
  block._version = stream.readLE<uint32_t>();
  block._previousBlock = stream.readSlice(SHA256_HASH_SIZE).reverse();
  block._merkleRoot = stream.readSlice(SHA256_HASH_SIZE).reverse();
  block._timestamp = stream.readLE<uint32_t>();
  block._bits = stream.readLE<uint32_t>();
  block._nonce = stream.readLE<uint32_t>();
  return block;
}

BtcBlock BtcBlock::fromVbkEncoding(ReadStream& stream) {
  ReadStream valStream(
      readSingleByteLenValue(stream, BTC_HEADER_SIZE, BTC_HEADER_SIZE));
  return BtcBlock::fromRaw(valStream);
}

void BtcBlock::toRaw(WriteStream& stream) const {
  stream.writeLE<uint32_t>(_version);
  stream.write(_previousBlock.reverse());
  stream.write(_merkleRoot.reverse());
  stream.writeLE<uint32_t>(_timestamp);
  stream.writeLE<uint32_t>(_bits);
  stream.writeLE<uint32_t>(_nonce);
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

BtcBlock BtcBlock::fromHex(const std::string& hex) {
  auto v = ParseHex(hex);
  return BtcBlock::fromRaw(v);
}

std::string BtcBlock::toHex() const { return HexStr(this->toRaw()); }

std::vector<uint8_t> BtcBlock::toRaw() const {
  WriteStream stream;
  this->toRaw(stream);
  return stream.data();
}

std::string BtcBlock::toPrettyString() const {
  return fmt::sprintf(
      "BtcBlock{version=%lu, prev=%s, merkleRoot=%s, timestamp=%lu, "
      "bits=%lu, nonce=%lu}",
      _version,
      _previousBlock.toHex(),
      _merkleRoot.toHex(),
      _timestamp,
      _bits,
      _nonce);
}

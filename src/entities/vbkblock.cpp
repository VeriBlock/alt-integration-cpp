// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/vbkblock.hpp"

using namespace altintegration;

VbkBlock VbkBlock::fromRaw(Slice<const uint8_t> bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

VbkBlock VbkBlock::fromRaw(ReadStream& stream) {
  VbkBlock block{};
  block._height = stream.readBE<int32_t>();
  block._version = stream.readBE<int16_t>();
  block._previousBlock = stream.readSlice(VBLAKE_PREVIOUS_BLOCK_HASH_SIZE);
  block._previousKeystone = stream.readSlice(VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE);
  block._secondPreviousKeystone =
      stream.readSlice(VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE);
  block._merkleRoot = stream.readSlice(VBK_MERKLE_ROOT_HASH_SIZE);
  block._timestamp = stream.readBE<int32_t>();
  block._difficulty = stream.readBE<int32_t>();
  block._nonce = stream.readBE<int32_t>();
  return block;
}

VbkBlock VbkBlock::fromVbkEncoding(ReadStream& stream) {
  auto blockBytes =
      readSingleByteLenValue(stream, VBK_HEADER_SIZE, VBK_HEADER_SIZE);
  ReadStream blockStream(blockBytes);
  return VbkBlock::fromRaw(blockStream);
}

VbkBlock VbkBlock::fromVbkEncoding(const std::string& bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

void VbkBlock::toVbkEncoding(WriteStream& stream) const {
  WriteStream blockStream;
  toRaw(blockStream);
  writeSingleByteLenValue(stream, blockStream.data());
}

std::vector<uint8_t> VbkBlock::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

uint32_t VbkBlock::getDifficulty() const { return _difficulty; }

uint32_t VbkBlock::getBlockTime() const { return _timestamp; }

VbkBlock::hash_t VbkBlock::getHash() const {
  WriteStream stream;
  toRaw(stream);
  return vblake(stream.data());
}

VbkBlock::short_hash_t VbkBlock::getShortHash() const {
  return getHash().trimLE<VbkBlock::short_hash_t::size()>();
}

VbkBlock VbkBlock::fromHex(const std::string& hex) {
  auto v = ParseHex(hex);
  return VbkBlock::fromRaw(v);
}

std::string VbkBlock::toHex() const { return HexStr(toRaw()); }

void VbkBlock::toRaw(WriteStream& stream) const {
  stream.writeBE<int32_t>(_height);
  stream.writeBE<int16_t>(_version);
  stream.write(_previousBlock);
  stream.write(_previousKeystone);
  stream.write(_secondPreviousKeystone);
  stream.write(_merkleRoot);
  stream.writeBE<int32_t>(_timestamp);
  stream.writeBE<int32_t>(_difficulty);
  stream.writeBE<int32_t>(_nonce);
}

std::vector<uint8_t> VbkBlock::toRaw() const {
  WriteStream stream;
  toRaw(stream);
  return stream.data();
}

std::string VbkBlock::toPrettyString() const {
  return fmt::sprintf(
      "VbkBlock{height=%ld, version=%d, prev=%s, ks1=%s, "
      "ks2=%s, mroot=%s, timestamp=%ld, "
      "diff=%ld, nonce=%ld}",
      _height,
      _version,
      _previousBlock.toHex(),
      _previousKeystone.toHex(),
      _secondPreviousKeystone.toHex(),
      _merkleRoot.toHex(),
      _timestamp,
      _difficulty,
      _nonce);
}

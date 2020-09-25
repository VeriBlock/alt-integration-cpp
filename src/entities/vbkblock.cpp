// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/vbkblock.hpp"

using namespace altintegration;

const std::string VbkBlock::_name = "VBK";

VbkBlock VbkBlock::fromRaw(Slice<const uint8_t> bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

VbkBlock VbkBlock::fromRaw(ReadStream& stream) {
  VbkBlock block{};
  block.height = stream.readBE<int32_t>();
  block.version = stream.readBE<int16_t>();
  block.previousBlock = stream.readSlice(VBLAKE_PREVIOUS_BLOCK_HASH_SIZE);
  block.previousKeystone = stream.readSlice(VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE);
  block.secondPreviousKeystone =
      stream.readSlice(VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE);
  block.merkleRoot = stream.readSlice(VBK_MERKLE_ROOT_HASH_SIZE);
  block.timestamp = stream.readBE<int32_t>();
  block.difficulty = stream.readBE<int32_t>();
  block.nonce = stream.readBE<int32_t>();
  return block;
}

VbkBlock VbkBlock::fromVbkEncoding(ReadStream& stream) {
  auto blockBytes =
      readSingleByteLenValue(stream, VBK_HEADER_SIZE_VBLAKE, VBK_HEADER_SIZE_VBLAKE);
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

uint32_t VbkBlock::getDifficulty() const { return difficulty; }

uint32_t VbkBlock::getBlockTime() const { return timestamp; }

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
  stream.writeBE<int32_t>(height);
  stream.writeBE<int16_t>(version);
  stream.write(previousBlock);
  stream.write(previousKeystone);
  stream.write(secondPreviousKeystone);
  stream.write(merkleRoot);
  stream.writeBE<int32_t>(timestamp);
  stream.writeBE<int32_t>(difficulty);
  stream.writeBE<int32_t>(nonce);
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
      height,
      version,
      previousBlock.toHex(),
      previousKeystone.toHex(),
      secondPreviousKeystone.toHex(),
      merkleRoot.toHex(),
      timestamp,
      difficulty,
      nonce);
}

bool altintegration::DeserializeRaw(ReadStream& stream,
                                    VbkBlock& out,
                                    ValidationState& state) {
  VbkBlock block{};
  if (!stream.readBE<int32_t>(block.height, state)) {
    return state.Invalid("vbk-block-height");
  }
  if (!stream.readBE<int16_t>(block.version, state)) {
    return state.Invalid("vbk-block-version");
  }
  Slice<const uint8_t> previousBlock;
  if (!stream.readSlice(
          VBLAKE_PREVIOUS_BLOCK_HASH_SIZE, previousBlock, state)) {
    return state.Invalid("vbk-block-previous");
  }
  block.previousBlock = previousBlock;
  Slice<const uint8_t> previousKeystone;
  if (!stream.readSlice(
          VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE, previousKeystone, state)) {
    return state.Invalid("vbk-block-previous-keystone");
  }
  block.previousKeystone = previousKeystone;
  Slice<const uint8_t> secondPreviousKeystone;
  if (!stream.readSlice(
          VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE, secondPreviousKeystone, state)) {
    return state.Invalid("vbk-block-second-previous-keystone");
  }
  block.secondPreviousKeystone = secondPreviousKeystone;
  Slice<const uint8_t> merkleRoot;
  if (!stream.readSlice(VBK_MERKLE_ROOT_HASH_SIZE, merkleRoot, state)) {
    return state.Invalid("vbk-block-merkle-root");
  }
  block.merkleRoot = merkleRoot;
  if (!stream.readBE<int32_t>(block.timestamp, state)) {
    return state.Invalid("vbk-block-timestamp");
  }
  if (!stream.readBE<int32_t>(block.difficulty, state)) {
    return state.Invalid("vbk-block-difficulty");
  }
  if (!stream.readBE<int32_t>(block.nonce, state)) {
    return state.Invalid("vbk-block-nonce");
  }
  out = block;
  return true;
}

bool altintegration::Deserialize(ReadStream& stream,
                                 VbkBlock& out,
                                 ValidationState& state) {
  Slice<const uint8_t> value;
  if (!readSingleByteLenValue(
          stream, value, state,
                              VBK_HEADER_SIZE_VBLAKE,
                              VBK_HEADER_SIZE_VBLAKE)) {
    return state.Invalid("vbk-block-bad-header");
  }
  return DeserializeRaw(value, out, state);
}

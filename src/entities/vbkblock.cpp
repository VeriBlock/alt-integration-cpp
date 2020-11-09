// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/vbkblock.hpp"

#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/config.hpp>

#include "veriblock/crypto/progpow.hpp"

namespace altintegration {

const std::string VbkBlock::_name = "VBK";

VbkBlock VbkBlock::fromRaw(Slice<const uint8_t> bytes,
                           const hash_t& precalculatedHash) {
  ReadStream stream(bytes);
  return fromRaw(stream, precalculatedHash);
}

VbkBlock VbkBlock::fromRaw(ReadStream& stream,
                           const hash_t& precalculatedHash) {
  try {
    VbkBlock block{};
    block.height = stream.readBE<int32_t>();
    block.version = stream.readBE<int16_t>();
    block.previousBlock = stream.readSlice(VBLAKE_PREVIOUS_BLOCK_HASH_SIZE);
    block.previousKeystone =
        stream.readSlice(VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE);
    block.secondPreviousKeystone =
        stream.readSlice(VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE);
    block.merkleRoot = stream.readSlice(VBK_MERKLE_ROOT_HASH_SIZE);
    block.timestamp = stream.readBE<int32_t>();
    block.difficulty = stream.readBE<int32_t>();
    block.nonce = stream.readBE<uint64_t>(5);
    block.hash_ = precalculatedHash;
    return block;
  } catch (const std::exception& e) {
    throw std::invalid_argument(
        fmt::format("Can not deserialize VBK block ({}) from {}",
                    e.what(),
                    HexStr(stream.remainingBytes())));
  }
}

VbkBlock VbkBlock::fromVbkEncoding(ReadStream& stream) {
  auto blockBytes = readSingleByteLenValue(
      stream, VBK_HEADER_SIZE_PROGPOW, VBK_HEADER_SIZE_PROGPOW);
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

VbkBlock::hash_t VbkBlock::calculateHash() const {
  WriteStream stream;
  toRaw(stream);
  auto& header = stream.data();
  return progPowHash(header);
}

VbkBlock::hash_t VbkBlock::getHash() const {
  static hash_t empty{};
  if (hash_ == empty) {
    hash_ = calculateHash();
  }

  return hash_;
}

VbkBlock::short_hash_t VbkBlock::getShortHash() const {
  return getHash().trimLE<VbkBlock::short_hash_t::size()>();
}

VbkBlock VbkBlock::fromHex(const std::string& hex,
                           const hash_t& precalculatedHash) {
  auto v = ParseHex(hex);
  return VbkBlock::fromRaw(v, precalculatedHash);
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
  stream.writeBE<uint64_t>(nonce, 5);
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

VbkBlock::VbkBlock(int32_t h,
                   int16_t v,
                   uint96 prevBlock,
                   VbkBlock::keystone_t prev1,
                   VbkBlock::keystone_t prev2,
                   uint128 mroot,
                   int32_t ts,
                   int32_t diff,
                   uint64_t nonce)
    : height(h),
      version(v),
      previousBlock(std::move(prevBlock)),
      previousKeystone(std::move(prev1)),
      secondPreviousKeystone(std::move(prev2)),
      merkleRoot(std::move(mroot)),
      timestamp(ts),
      difficulty(diff),
      nonce(nonce) {}

bool operator==(const VbkBlock& a, const VbkBlock& b) {
  return a.height == b.height && a.difficulty == b.difficulty &&
         a.timestamp == b.timestamp && a.version == b.version &&
         a.merkleRoot == b.merkleRoot && a.previousBlock == b.previousBlock &&
         a.previousKeystone == b.previousKeystone &&
         a.secondPreviousKeystone == b.secondPreviousKeystone &&
         a.nonce == b.nonce;
}

bool operator!=(const VbkBlock& a, const VbkBlock& b) { return !(a == b); }

void VbkBlock::setNonce(uint64_t nnc) {
  nonce = nnc;
  invalidateHash();
}

bool DeserializeRaw(ReadStream& stream, VbkBlock& out, ValidationState& state) {
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
  if (!stream.readBE<uint64_t>(block.nonce, state, 5)) {
    return state.Invalid("vbk-block-nonce");
  }
  out = block;
  return true;
}
void VbkBlock::setHeight(int32_t h) {
  height = h;
  invalidateHash();
}
void VbkBlock::setVersion(int16_t v) {
  version = v;
  invalidateHash();
}
void VbkBlock::setPreviousBlock(const uint96& prev) {
  previousBlock = prev;
  invalidateHash();
}
void VbkBlock::setPreviousKeystone(const VbkBlock::keystone_t& ks) {
  previousKeystone = ks;
  invalidateHash();
}
void VbkBlock::setSecondPreviousKeystone(const VbkBlock::keystone_t& ks) {
  secondPreviousKeystone = ks;
  invalidateHash();
}
void VbkBlock::setMerkleRoot(const uint128& mroot) {
  merkleRoot = mroot;
  invalidateHash();
}
void VbkBlock::setTimestamp(int32_t ts) {
  timestamp = ts;
  invalidateHash();
}
void VbkBlock::setDifficulty(int32_t diff) {
  difficulty = diff;
  invalidateHash();
}

bool Deserialize(ReadStream& stream, VbkBlock& out, ValidationState& state) {
  Slice<const uint8_t> value;
  if (!readSingleByteLenValue(stream,
                              value,
                              state,
                              VBK_HEADER_SIZE_PROGPOW,
                              VBK_HEADER_SIZE_PROGPOW)) {
    return state.Invalid("vbk-block-bad-header");
  }
  return DeserializeRaw(value, out, state);
}

}  // namespace altintegration
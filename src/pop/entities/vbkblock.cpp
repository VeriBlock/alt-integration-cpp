// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/blockchain/vbk_chain_params.hpp>
#include <veriblock/pop/config.hpp>
#include <veriblock/pop/crypto/progpow.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>

namespace altintegration {

// NOLINTNEXTLINE(cert-err58-cpp)
const std::string VbkBlock::_name = "VBK";

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

size_t VbkBlock::estimateSize() const {
  size_t rawSize = 0;
  rawSize += sizeof(height);
  rawSize += sizeof(version);
  rawSize += previousBlock.size();
  rawSize += previousKeystone.size();
  rawSize += secondPreviousKeystone.size();
  rawSize += merkleRoot.size();
  rawSize += sizeof(timestamp);
  rawSize += sizeof(difficulty);
  rawSize += 5;  // nonce

  size_t size = 0;
  size += singleByteLenValueSize(rawSize);
  return size;
}

VbkBlock::hash_t VbkBlock::calculateHash() const {
  WriteStream stream;
  toRaw(stream);
  auto& header = stream.data();
  return progPowHash(header);
}

const VbkBlock::hash_t& VbkBlock::getHash() const {
  static hash_t empty{};
  if (hash_ == empty) {
    hash_ = calculateHash();
  }

  return hash_;
}

VbkBlock::short_hash_t VbkBlock::getShortHash() const {
  return getHash().trimLE<VbkBlock::short_hash_t::size()>();
}

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

std::string VbkBlock::toShortPrettyString() const {
  return format("VBK(height={}, hash={})", height, getHash().toHex());
}

std::string VbkBlock::toPrettyString() const {
  return format(
      "VbkBlock{{height={}, version={}, prev={}, ks1={}, "
      "ks2={}, mroot={}, timestamp={}, "
      "diff={}, nonce={}}}",
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
void VbkBlock::setTimestamp(uint32_t ts) {
  timestamp = ts;
  invalidateHash();
}
void VbkBlock::setDifficulty(int32_t diff) {
  difficulty = diff;
  invalidateHash();
}

void setPrecalculatedHash(VbkBlock& block,
                          const VbkBlock::hash_t& precalculatedHash) {
  block.hash_ = precalculatedHash;
}

bool DeserializeFromRaw(ReadStream& stream,
                        VbkBlock& block,
                        ValidationState& state,
                        const VbkBlock::hash_t& hash) {
  if (!stream.readBE<int32_t>(block.height, state)) {
    return state.Invalid("vbk-block-height");
  }
  if (!stream.readBE<int16_t>(block.version, state)) {
    return state.Invalid("vbk-block-version");
  }
  Slice<const uint8_t> previousBlock;
  if (!stream.readSlice(VBK_PREVIOUS_BLOCK_HASH_SIZE, previousBlock, state)) {
    return state.Invalid("vbk-block-previous");
  }
  block.previousBlock = previousBlock;
  Slice<const uint8_t> previousKeystone;
  if (!stream.readSlice(
          VBK_PREVIOUS_KEYSTONE_HASH_SIZE, previousKeystone, state)) {
    return state.Invalid("vbk-block-keystone1");
  }
  block.previousKeystone = previousKeystone;
  Slice<const uint8_t> secondPreviousKeystone;
  if (!stream.readSlice(
          VBK_PREVIOUS_KEYSTONE_HASH_SIZE, secondPreviousKeystone, state)) {
    return state.Invalid("vbk-block-keystone2");
  }
  block.secondPreviousKeystone = secondPreviousKeystone;
  Slice<const uint8_t> merkleRoot;
  if (!stream.readSlice(VBK_MERKLE_ROOT_HASH_SIZE, merkleRoot, state)) {
    return state.Invalid("vbk-block-merkle-root");
  }
  block.merkleRoot = merkleRoot;
  if (!stream.readBE<uint32_t>(block.timestamp, state)) {
    return state.Invalid("vbk-block-timestamp");
  }
  if (!stream.readBE<int32_t>(block.difficulty, state)) {
    return state.Invalid("vbk-block-difficulty");
  }
  // VBK block nonce is 5 bytes after ProgPow fork
  if (!stream.readBE<uint64_t>(block.nonce, state, 5)) {
    return state.Invalid("vbk-block-nonce");
  }
  block.hash_ = hash;
  return true;
}

bool DeserializeFromRaw(ReadStream& stream,
                        VbkBlock& block,
                        ValidationState& state,
                        const AltChainParams& /*ignore*/,
                        const VbkBlock::hash_t& hash) {
  return DeserializeFromRaw(stream, block, state, hash);
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                VbkBlock& out,
                                ValidationState& state,
                                const VbkBlock::hash_t& precalculatedHash) {
  Slice<const uint8_t> value;
  if (!readSingleByteLenValue(stream,
                              value,
                              state,
                              VBK_HEADER_SIZE_PROGPOW,
                              VBK_HEADER_SIZE_PROGPOW)) {
    return state.Invalid("vbk-block-bad-header");
  }

  ReadStream s(value);
  return DeserializeFromRaw(s, out, state, precalculatedHash);
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                VbkBlock& out,
                                ValidationState& state,
                                const AltChainParams& /*ignore*/,
                                const VbkBlock::hash_t& precalculatedHash) {
  return DeserializeFromVbkEncoding(stream, out, state, precalculatedHash);
}

}  // namespace altintegration
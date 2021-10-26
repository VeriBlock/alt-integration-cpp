// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/entities/btcblock.hpp>

namespace altintegration {

// NOLINTNEXTLINE(cert-err58-cpp)
const std::string BtcBlock::_name = "BTC";

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

size_t BtcBlock::estimateSize() const {
  size_t rawSize = 0;
  rawSize += sizeof(version);
  rawSize += previousBlock.size();
  rawSize += merkleRoot.size();
  rawSize += sizeof(timestamp);
  rawSize += sizeof(bits);
  rawSize += sizeof(nonce);

  size_t size = 0;
  size += singleByteLenValueSize(rawSize);

  return size;
}

BtcBlock::hash_t BtcBlock::calculateHash() const {
  WriteStream stream;
  toRaw(stream);
  return sha256twice(stream.data()).reverse();
}

const BtcBlock::hash_t& BtcBlock::getHash() const {
  static uint256 empty;
  if (hash_ == empty) {
    hash_ = calculateHash();
  }

  return hash_;
}

std::string BtcBlock::toPrettyString() const {
  return format(
      "BtcBlock{{version={}, prev={}, merkleRoot={}, timestamp={}, "
      "bits={}, nonce={}}}",
      version,
      previousBlock.toHex(),
      merkleRoot.toHex(),
      timestamp,
      bits,
      nonce);
}

void BtcBlock::setTimestamp(uint32_t ts) {
  timestamp = ts;
  invalidateHash();
}

void BtcBlock::setNonce(uint32_t nnc) {
  nonce = nnc;
  invalidateHash();
}

void BtcBlock::setVersion(uint32_t v) {
  version = v;
  invalidateHash();
}
void BtcBlock::setPreviousBlock(const uint256& prev) {
  previousBlock = prev;
  invalidateHash();
}
void BtcBlock::setMerkleRoot(const uint256& mr) {
  merkleRoot = mr;
  invalidateHash();
}
void BtcBlock::setDifficulty(uint32_t diff) {
  bits = diff;
  invalidateHash();
}

bool DeserializeFromRaw(ReadStream& stream,
                        BtcBlock& block,
                        ValidationState& state,
                        const BtcBlock::hash_t& precalculatedHash) {
  if (!stream.readLE<uint32_t>(block.version, state)) {
    return state.Invalid("btc-block-version");
  }
  Slice<const uint8_t> previousBlock;
  if (!stream.readSlice(SHA256_HASH_SIZE, previousBlock, state)) {
    return state.Invalid("btc-block-previous");
  }
  block.previousBlock = previousBlock.reverse();
  Slice<const uint8_t> merkleRoot;
  if (!stream.readSlice(SHA256_HASH_SIZE, merkleRoot, state)) {
    return state.Invalid("btc-block-merkle-root");
  }
  block.merkleRoot = merkleRoot.reverse();
  if (!stream.readLE<uint32_t>(block.timestamp, state)) {
    return state.Invalid("btc-block-timestamp");
  }
  if (!stream.readLE<uint32_t>(block.bits, state)) {
    return state.Invalid("btc-block-difficulty");
  }
  if (!stream.readLE<uint32_t>(block.nonce, state)) {
    return state.Invalid("btc-block-nonce");
  }

  block.hash_ = precalculatedHash;
  return true;
}

bool DeserializeFromRaw(ReadStream& stream,
                        BtcBlock& block,
                        ValidationState& state,
                        const AltChainParams& /*ignore*/,
                        const BtcBlock::hash_t& precalculatedHash) {
  return DeserializeFromRaw(stream, block, state, precalculatedHash);
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                BtcBlock& out,
                                ValidationState& state,
                                const BtcBlock::hash_t& precalculatedHash) {
  Slice<const uint8_t> value;
  if (!readSingleByteLenValue(
          stream, value, state, BTC_HEADER_SIZE, BTC_HEADER_SIZE)) {
    return state.Invalid("btc-block-bad-header");
  }
  out.hash_ = precalculatedHash;
  return DeserializeFromRaw(value, out, state);
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                BtcBlock& out,
                                ValidationState& state,
                                const AltChainParams& /*ignore*/,
                                const BtcBlock::hash_t& precalculatedHash) {
  return DeserializeFromVbkEncoding(stream, out, state, precalculatedHash);
}

BtcBlock::BtcBlock(uint32_t version,
                   uint256 previousBlock,
                   uint256 merkleRoot,
                   uint32_t timestamp,
                   uint32_t bits,
                   uint32_t nonce)
    : version(version),
      previousBlock(std::move(previousBlock)),
      merkleRoot(std::move(merkleRoot)),
      timestamp(timestamp),
      bits(bits),
      nonce(nonce) {}

}  // namespace altintegration
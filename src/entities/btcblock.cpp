// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/entities/btcblock.hpp>

namespace altintegration {

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

uint32_t BtcBlock::getDifficulty() const { return bits; }

uint32_t BtcBlock::getBlockTime() const { return timestamp; }

uint256 BtcBlock::getHash() const {
  WriteStream stream;
  toRaw(stream);
  return sha256twice(stream.data()).reverse();
}

std::string BtcBlock::toPrettyString() const {
  return fmt::sprintf(
      "BtcBlock{version=%lu, prev=%s, merkleRoot=%s, timestamp=%lu, "
      "bits=%lu, nonce=%lu}",
      version,
      previousBlock.toHex(),
      merkleRoot.toHex(),
      timestamp,
      bits,
      nonce);
}

bool DeserializeFromRaw(ReadStream& stream,
                        BtcBlock& block,
                        ValidationState& state,
                        const BtcBlock::hash_t& /*ignore*/) {
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
  return true;
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                BtcBlock& out,
                                ValidationState& state,
                                const BtcBlock::hash_t& /*ignore*/) {
  Slice<const uint8_t> value;
  if (!readSingleByteLenValue(
          stream, value, state, BTC_HEADER_SIZE, BTC_HEADER_SIZE)) {
    return state.Invalid("btc-block-bad-header");
  }
  return DeserializeFromRaw(value, out, state);
}

}  // namespace altintegration
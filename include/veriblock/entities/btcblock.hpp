#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_

#include <cstdint>

#include "veriblock/entities/hashes.hpp"
#include "veriblock/serde.hpp"

namespace VeriBlock {

struct BtcBlock {
  uint32_t version;
  Sha256Hash previousBlock;
  Sha256Hash merkleRoot;
  uint32_t timestamp;
  uint32_t bits;
  uint32_t nonce;

  static BtcBlock fromRaw(ReadStream& stream) {
    BtcBlock block{};
    block.version = stream.readLE<uint32_t>();
    block.previousBlock = stream.readSlice(SHA256_HASH_SIZE);
    block.merkleRoot = stream.readSlice(SHA256_HASH_SIZE);
    block.timestamp = stream.readLE<uint32_t>();
    block.bits = stream.readLE<uint32_t>();
    block.nonce = stream.readLE<uint32_t>();
    return block;
  }

  static BtcBlock fromVbkEncoding(ReadStream& stream) {
    ReadStream valStream(
        readSingleByteLenValue(stream, BTC_HEADER_SIZE, BTC_HEADER_SIZE));
    return BtcBlock::fromRaw(valStream);
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_

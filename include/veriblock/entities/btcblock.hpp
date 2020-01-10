#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_

#include <cstdint>

#include "sha256hash.hpp"
#include "veriblock/read_stream.hpp"

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
    block.previousBlock = Sha256Hash::fromRaw(stream);
    block.merkleRoot = Sha256Hash::fromRaw(stream);
    block.timestamp = stream.readLE<uint32_t>();
    block.bits = stream.readLE<uint32_t>();
    block.nonce = stream.readLE<uint32_t>();
    return block;
  }

  static BtcBlock fromVbk(ReadStream& stream) {
    const auto bytes =
        readSingleByteLenValue(stream, BTC_HEADER_SIZE, BTC_HEADER_SIZE);
    ReadStream s(bytes);
    return BtcBlock::fromRaw(s);
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_

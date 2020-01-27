#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_

#include <algorithm>
#include <cstdint>

#include "veriblock/uint.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"

namespace VeriBlock {

struct BtcBlock {
  uint32_t version{};
  uint256 previousBlock{};
  uint256 merkleRoot{};
  uint32_t timestamp{};
  uint32_t bits{};
  uint32_t nonce{};

  static BtcBlock fromRaw(ReadStream& stream) {
    BtcBlock block{};
    block.version = stream.readLE<uint32_t>();
    block.previousBlock = stream.readSlice(SHA256_HASH_SIZE).reverse();
    block.merkleRoot = stream.readSlice(SHA256_HASH_SIZE).reverse();
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

  void toRaw(WriteStream& stream) const {
    stream.writeLE<uint32_t>(version);
    stream.write(previousBlock.reverse());
    stream.write(merkleRoot.reverse());
    stream.writeLE<uint32_t>(timestamp);
    stream.writeLE<uint32_t>(bits);
    stream.writeLE<uint32_t>(nonce);
  }

  void toVbkEncoding(WriteStream& stream) const {
    WriteStream blockStream;
    toRaw(blockStream);
    writeSingleByteLenValue(stream, blockStream.data());
  }

  uint256 getHash() const {
    WriteStream stream;
    toRaw(stream);
    return sha256twice(stream.data());
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_

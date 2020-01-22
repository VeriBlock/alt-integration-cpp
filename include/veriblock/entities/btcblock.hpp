#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_

#include <algorithm>
#include <cstdint>

#include "veriblock/entities/hashes.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/sha256.h"

namespace VeriBlock {

struct BtcBlock {
  uint32_t version{};
  Sha256Hash previousBlock{};
  Sha256Hash merkleRoot{};
  uint32_t timestamp{};
  uint32_t bits{};
  uint32_t nonce{};

  static BtcBlock fromRaw(ReadStream& stream) {
    BtcBlock block{};
    block.version = stream.readLE<uint32_t>();
    block.previousBlock =
        ((Sha256Hash)stream.readSlice(SHA256_HASH_SIZE)).reverse();
    block.merkleRoot =
        ((Sha256Hash)stream.readSlice(SHA256_HASH_SIZE)).reverse();
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

  Sha256Hash getBlockHash() const {
    Sha256Hash hash;

    WriteStream stream;
    toRaw(stream);

    sha256(hash.data(), stream.data().data(), (uint32_t)stream.data().size());
    sha256(hash.data(), hash.data(), SHA256_HASH_SIZE);
    std::reverse(hash.begin(), hash.end());
    return hash;
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_

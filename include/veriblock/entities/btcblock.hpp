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

  BtcBlock(uint32_t _version,
           Sha256Hash _previousBlock,
           Sha256Hash _merkleRoot,
           uint32_t _timestamp,
           uint32_t _bits,
           uint32_t _nonce)
      : version(_version),
        previousBlock(_previousBlock),
        merkleRoot(_merkleRoot),
        timestamp(_timestamp),
        bits(_bits),
        nonce(_nonce) {}

  static BtcBlock fromRaw(ReadStream& stream) {
    uint32_t version = stream.readLE<uint32_t>();
    Sha256Hash previousBlock = stream.readSlice(SHA256_HASH_SIZE);
    Sha256Hash merkleRoot = stream.readSlice(SHA256_HASH_SIZE);
    uint32_t timestamp = stream.readLE<uint32_t>();
    uint32_t bits = stream.readLE<uint32_t>();
    uint32_t nonce = stream.readLE<uint32_t>();
    return BtcBlock(version, previousBlock, merkleRoot, timestamp, bits, nonce);
  }

  static BtcBlock fromVbkEncoding(ReadStream& stream) {
    ReadStream valStream(
        readSingleByteLenValue(stream, BTC_HEADER_SIZE, BTC_HEADER_SIZE));
    return BtcBlock::fromRaw(valStream);
  }

  void toRaw(WriteStream& stream) const {
    stream.writeLE<uint32_t>(version);
    stream.write(previousBlock);
    stream.write(merkleRoot);
    stream.writeLE<uint32_t>(timestamp);
    stream.writeLE<uint32_t>(bits);
    stream.writeLE<uint32_t>(nonce);
  }

  void toVbkEncoding(WriteStream& stream) const {
    WriteStream blockStream;
    toRaw(blockStream);
    writeSingleByteLenValue(stream, blockStream.data());
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCBLOCK_HPP_

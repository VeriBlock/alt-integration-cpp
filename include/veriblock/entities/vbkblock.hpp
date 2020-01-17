#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_

#include <cstdint>

#include "veriblock/serde.hpp"

#include "veriblock/entities/hashes.hpp"

namespace VeriBlock {

struct VbkBlock {
  int32_t height;
  int16_t version;
  VBlakeBlockHash previousBlock;
  VBlakePrevKeystoneHash previousKeystone;
  VBlakePrevKeystoneHash secondPreviousKeystone;
  VbkMerkleRootSha256Hash merkleRoot;
  int32_t timestamp;
  int32_t difficulty;
  int32_t nonce;

  VbkBlock(int32_t _height,
           int16_t _version,
           VBlakeBlockHash _previousBlock,
           VBlakePrevKeystoneHash _previousKeystone,
           VBlakePrevKeystoneHash _secondPreviousKeystone,
           VbkMerkleRootSha256Hash _merkleRoot,
           int32_t _timestamp,
           int32_t _difficulty,
           int32_t _nonce)
      : height(_height),
        version(_version),
        previousBlock(_previousBlock),
        previousKeystone(_previousKeystone),
        secondPreviousKeystone(_secondPreviousKeystone),
        merkleRoot(_merkleRoot),
        timestamp(_timestamp),
        difficulty(_difficulty),
        nonce(_nonce) {}

  static VbkBlock fromRaw(ReadStream& stream) {
    int32_t height = stream.readBE<int32_t>();
    int16_t version = stream.readBE<int16_t>();
    VBlakeBlockHash previousBlock =
        stream.readSlice(VBLAKE_PREVIOUS_BLOCK_SIZE);
    VBlakePrevKeystoneHash previousKeystone =
        stream.readSlice(VBLAKE_PREVIOUS_KEYSTONE_SIZE);
    VBlakePrevKeystoneHash secondPreviousKeystone =
        stream.readSlice(VBLAKE_PREVIOUS_KEYSTONE_SIZE);
    VbkMerkleRootSha256Hash merkleRoot = stream.readSlice(VBK_MERKLE_ROOT_SIZE);
    int32_t timestamp = stream.readBE<int32_t>();
    int32_t difficulty = stream.readBE<int32_t>();
    int32_t nonce = stream.readBE<int32_t>();
    return VbkBlock(height,
                    version,
                    previousBlock,
                    previousKeystone,
                    secondPreviousKeystone,
                    merkleRoot,
                    timestamp,
                    difficulty,
                    nonce);
  }

  static VbkBlock fromVbkEncoding(ReadStream& stream) {
    auto blockBytes =
        readSingleByteLenValue(stream, VBK_HEADER_SIZE, VBK_HEADER_SIZE);
    ReadStream blockStream(blockBytes);
    return VbkBlock::fromRaw(blockStream);
  }

  void toRaw(WriteStream& stream) const {
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

  void toVbkEncoding(WriteStream& stream) const {
    WriteStream blockStream;
    toRaw(blockStream);
    writeSingleByteLenValue(stream, blockStream.data());
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_

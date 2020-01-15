#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_

#include <cstdint>

#include "veriblock/entities/hashes.hpp"
#include "veriblock/serde.hpp"

namespace VeriBlock {

struct VbkBlock {
  int32_t height{};
  int16_t version{};
  VBlakeBlockHash previousBlock{};
  VBlakePrevKeystoneHash previousKeystone{};
  VBlakePrevKeystoneHash secondPreviousKeystone{};
  VbkMerkleRootSha256Hash merkleRoot{};
  int32_t timestamp{};
  int32_t difficulty{};
  int32_t nonce{};

  static VbkBlock fromRaw(ReadStream& stream) {
    VbkBlock block{};
    block.height = stream.readBE<int32_t>();
    block.version = stream.readBE<int16_t>();
    block.previousBlock = stream.readSlice(VBLAKE_PREVIOUS_BLOCK_SIZE);
    block.previousKeystone = stream.readSlice(VBLAKE_PREVIOUS_KEYSTONE_SIZE);
    block.secondPreviousKeystone =
        stream.readSlice(VBLAKE_PREVIOUS_KEYSTONE_SIZE);
    block.merkleRoot = stream.readSlice(VBK_MERKLE_ROOT_SIZE);
    block.timestamp = stream.readBE<int32_t>();
    block.difficulty = stream.readBE<int32_t>();
    block.nonce = stream.readBE<int32_t>();
    return block;
  }

  static VbkBlock fromVbkEncoding(ReadStream& stream) {
    auto blockBytes =
        readSingleByteLenValue(stream, VBK_HEADER_SIZE, VBK_HEADER_SIZE);
    ReadStream blockStream(blockBytes);
    return VbkBlock::fromRaw(blockStream);
  }

  static void toRaw(WriteStream& stream, const VbkBlock& block) {
    stream.writeBE(block.height);
    stream.writeBE(block.version);
    stream.write(block.previousBlock);
    stream.write(block.previousKeystone);
    stream.write(block.secondPreviousKeystone);
    stream.write(block.merkleRoot);
    stream.writeBE(block.timestamp);
    stream.writeBE(block.difficulty);
    stream.writeBE(block.nonce);
  }

  static void toVbkEncoding(WriteStream& stream, const VbkBlock& block) {
    WriteStream blockStream;
    toRaw(blockStream, block);
    writeSingleByteLenValue(stream, blockStream.data());
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_

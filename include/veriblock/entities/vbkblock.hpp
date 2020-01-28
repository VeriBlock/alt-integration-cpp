#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace VeriBlock {

struct VbkBlock {
  int32_t height{};
  int16_t version{};
  uint144 previousBlock{};
  uint72 previousKeystone{};
  uint72 secondPreviousKeystone{};
  uint128 merkleRoot{};
  int32_t timestamp{};
  int32_t difficulty{};
  int32_t nonce{};

  static VbkBlock fromRaw(const std::vector<uint8_t>& bytes) {
    ReadStream stream(bytes);
    return fromRaw(stream);
  }

  static VbkBlock fromRaw(ReadStream& stream) {
    VbkBlock block{};
    block.height = stream.readBE<int32_t>();
    block.version = stream.readBE<int16_t>();
    block.previousBlock = stream.readSlice(VBLAKE_PREVIOUS_BLOCK_HASH_SIZE);
    block.previousKeystone =
        stream.readSlice(VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE);
    block.secondPreviousKeystone =
        stream.readSlice(VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE);
    block.merkleRoot = stream.readSlice(VBK_MERKLE_ROOT_HASH_SIZE);
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

  uint192 getHash() const {
    WriteStream stream;
    toRaw(stream);
    return vblake(stream.data()).reverse();
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKBLOCK_HPP_

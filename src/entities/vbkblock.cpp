#include "veriblock/entities/vbkblock.hpp"

using namespace VeriBlock;

VbkBlock VbkBlock::fromRaw(Slice<const uint8_t> bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

VbkBlock VbkBlock::fromRaw(ReadStream& stream) {
  VbkBlock block{};
  block.height = stream.readBE<int32_t>();
  block.version = stream.readBE<int16_t>();
  block.previousBlock = stream.readSlice(VBLAKE_PREVIOUS_BLOCK_HASH_SIZE);
  block.previousKeystone = stream.readSlice(VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE);
  block.secondPreviousKeystone =
      stream.readSlice(VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE);
  block.merkleRoot = stream.readSlice(VBK_MERKLE_ROOT_HASH_SIZE);
  block.timestamp = stream.readBE<int32_t>();
  block.difficulty = stream.readBE<int32_t>();
  block.nonce = stream.readBE<int32_t>();
  return block;
}

VbkBlock VbkBlock::fromRaw(const std::string& bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

VbkBlock VbkBlock::fromVbkEncoding(ReadStream& stream) {
  auto blockBytes =
      readSingleByteLenValue(stream, VBK_HEADER_SIZE, VBK_HEADER_SIZE);
  ReadStream blockStream(blockBytes);
  return VbkBlock::fromRaw(blockStream);
}

void VbkBlock::toRaw(WriteStream& stream) const {
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

std::string VbkBlock::toRaw() const {
  WriteStream stream;
  toRaw(stream);
  return std::string(reinterpret_cast<const char*>(stream.data().data()),
                     stream.data().size());
}

void VbkBlock::toVbkEncoding(WriteStream& stream) const {
  WriteStream blockStream;
  toRaw(blockStream);
  writeSingleByteLenValue(stream, blockStream.data());
}

uint32_t VbkBlock::getDifficulty() const { return difficulty; }

uint32_t VbkBlock::getBlockTime() const { return timestamp; }

uint192 VbkBlock::getHash() const {
  WriteStream stream;
  toRaw(stream);
  return vblake(stream.data());
}

VbkBlock VbkBlock::fromHex(const std::string& hex) {
  auto v = ParseHex(hex);
  return VbkBlock::fromRaw(v);
}

std::string VbkBlock::toHex() const {
  WriteStream stream;
  toRaw(stream);
  return HexStr(stream.data());
}

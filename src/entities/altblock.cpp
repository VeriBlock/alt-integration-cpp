// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/altblock.hpp"

namespace altintegration {

const std::string AltBlock::_name = "ALT";

AltBlock AltBlock::fromRaw(Slice<const uint8_t> bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

AltBlock AltBlock::fromRaw(ReadStream& stream) {
  return fromVbkEncoding(stream);
}

AltBlock AltBlock::fromVbkEncoding(ReadStream& stream) {
  AltBlock block;
  uint32_t hash_size = stream.readBE<uint32_t>();
  block.hash.resize(hash_size);
  for (uint32_t i = 0; i < hash_size; ++i) {
    block.hash[i] = stream.readBE<uint8_t>();
  }
  hash_size = stream.readBE<uint32_t>();
  block.previousBlock.resize(hash_size);
  for (uint32_t i = 0; i < hash_size; ++i) {
    block.previousBlock[i] = stream.readBE<uint8_t>();
  }
  block.height = stream.readBE<int32_t>();
  block.timestamp = stream.readBE<uint32_t>();

  return block;
}

AltBlock AltBlock::fromVbkEncoding(const std::string& bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

void AltBlock::toRaw(WriteStream& stream) const {
  return toVbkEncoding(stream);
}

std::vector<uint8_t> AltBlock::toRaw() const {
  WriteStream stream;
  toRaw(stream);
  return stream.data();
}

void AltBlock::toVbkEncoding(WriteStream& stream) const {
  stream.writeBE<uint32_t>((uint32_t)hash.size());
  for (size_t i = 0; i < hash.size(); ++i) {
    stream.writeBE<uint8_t>(hash[i]);
  }
  stream.writeBE<uint32_t>((uint32_t)previousBlock.size());
  for (size_t i = 0; i < previousBlock.size(); ++i) {
    stream.writeBE<uint8_t>(previousBlock[i]);
  }
  stream.writeBE<int32_t>(height);
  stream.writeBE<uint32_t>(timestamp);
}

std::vector<uint8_t> AltBlock::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

uint32_t AltBlock::getBlockTime() const noexcept { return timestamp; }

AltBlock::hash_t AltBlock::getHash() const { return hash; }

bool operator==(const AltBlock& a, const AltBlock& b) {
  return a.hash == b.hash;
}
bool operator!=(const AltBlock& a, const AltBlock& b) { return !(a == b); }

bool Deserialize(ReadStream& stream, AltBlock& out, ValidationState& state) {
  (void)stream;
  (void)state;
  AltBlock block;

  // TODO: implement

  out = block;
  return true;
}

}  // namespace altintegration
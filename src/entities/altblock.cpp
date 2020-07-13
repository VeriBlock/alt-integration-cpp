// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/altblock.hpp"

using namespace altintegration;

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
  block._hash.resize(hash_size);
  for (uint32_t i = 0; i < hash_size; ++i) {
    block._hash[i] = stream.readBE<uint8_t>();
  }
  hash_size = stream.readBE<uint32_t>();
  block._previousBlock.resize(hash_size);
  for (uint32_t i = 0; i < hash_size; ++i) {
    block._previousBlock[i] = stream.readBE<uint8_t>();
  }
  block._height = stream.readBE<int32_t>();
  block._timestamp = stream.readBE<uint32_t>();

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
  stream.writeBE<uint32_t>((uint32_t)_hash.size());
  for (size_t i = 0; i < _hash.size(); ++i) {
    stream.writeBE<uint8_t>(_hash[i]);
  }
  stream.writeBE<uint32_t>((uint32_t)_previousBlock.size());
  for (size_t i = 0; i < _previousBlock.size(); ++i) {
    stream.writeBE<uint8_t>(_previousBlock[i]);
  }
  stream.writeBE<int32_t>(_height);
  stream.writeBE<uint32_t>(_timestamp);
}

std::vector<uint8_t> AltBlock::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/altblock.hpp"

using namespace altintegration;

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

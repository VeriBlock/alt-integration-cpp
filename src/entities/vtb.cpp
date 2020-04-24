// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/vtb.hpp"

using namespace altintegration;

VTB VTB::fromVbkEncoding(ReadStream& stream) {
  VTB vtb{};
  vtb.transaction = VbkPopTx::fromVbkEncoding(stream);
  vtb.merklePath = VbkMerklePath::fromVbkEncoding(stream);
  vtb.containingBlock = VbkBlock::fromVbkEncoding(stream);
  vtb.context = readArrayOf<VbkBlock>(
      stream, 0, MAX_CONTEXT_COUNT, [](ReadStream& stream) {
        return VbkBlock::fromVbkEncoding(stream);
      });

  return vtb;
}

VTB VTB::fromVbkEncoding(Slice<const uint8_t> bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

VTB VTB::fromVbkEncoding(const std::string& bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

void VTB::toVbkEncoding(WriteStream& stream) const {
  WriteStream txStream;
  transaction.toVbkEncoding(stream);
  merklePath.toVbkEncoding(stream);
  containingBlock.toVbkEncoding(stream);
  writeSingleBEValue(stream, context.size());
  for (const auto& block : context) {
    block.toVbkEncoding(stream);
  }
}

std::vector<uint8_t> VTB::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

VTB::id_t VTB::getId() const {
  WriteStream stream;
  auto left = transaction.getHash();
  containingBlock.toVbkEncoding(stream);
  return sha256(left, stream.data());
}

VbkBlock VTB::getContainingBlock() const { return this->containingBlock; }

VbkBlock VTB::getEndorsedBlock() const {
  return this->transaction.publishedBlock;
}

VTB VTB::fromHex(const std::string& hex) {
  auto data = ParseHex(hex);
  return fromVbkEncoding(data);
}

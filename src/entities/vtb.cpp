#include "veriblock/entities/vtb.hpp"

using namespace VeriBlock;

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

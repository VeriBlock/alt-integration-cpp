#include "veriblock/entities/atv.hpp"

using namespace VeriBlock;

ATV ATV::fromVbkEncoding(ReadStream& stream) {
  ATV atv{};
  atv.transaction = VbkTx::fromVbkEncoding(stream);
  atv.merklePath = VbkMerklePath::fromVbkEncoding(stream);
  atv.containingBlock = VbkBlock::fromVbkEncoding(stream);
  atv.context = readArrayOf<VbkBlock>(
      stream, 0, MAX_CONTEXT_COUNT_ALT_PUBLICATION, [](ReadStream& stream) {
        return VbkBlock::fromVbkEncoding(stream);
      });

  return atv;
}

void ATV::toVbkEncoding(WriteStream& stream) const {
  WriteStream txStream;
  transaction.toVbkEncoding(stream);
  merklePath.toVbkEncoding(stream);
  containingBlock.toVbkEncoding(stream);
  writeSingleBEValue(stream, context.size());
  for (const auto& block : context) {
    block.toVbkEncoding(stream);
  }
}

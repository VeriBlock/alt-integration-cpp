#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/serde.hpp"
#include "veriblock/consts.hpp"

#include "veriblock/entities/vbktx.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbk_merkle_path.hpp"

namespace VeriBlock {

struct ATV {
  VbkTx transaction{};
  VbkMerklePath merklePath{};
  VbkBlock containingBlock{};
  std::vector<VbkBlock> context{};

  static ATV fromVbkEncoding(ReadStream& stream) {
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

  void toVbkEncoding(WriteStream& stream) const {
    WriteStream txStream;
    transaction.toVbkEncoding(stream);
    merklePath.toVbkEncoding(stream);
    containingBlock.toVbkEncoding(stream);
    writeSingleBEValue(stream, context.size());
    for (const auto& block : context) {
      block.toVbkEncoding(stream);
    }
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_

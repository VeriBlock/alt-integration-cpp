#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VTB_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VTB_HPP_

#include <vector>

#include "veriblock/serde.hpp"
#include "veriblock/consts.hpp"

#include "veriblock/entities/vbkpoptx.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbk_merkle_path.hpp"

namespace VeriBlock {

struct VTB {
  VbkPopTx transaction{};
  VbkMerklePath merklePath{};
  VbkBlock containingBlock{};
  std::vector<VbkBlock> context{};

  static VTB fromVbkEncoding(ReadStream& stream) {
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

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VTB_HPP_

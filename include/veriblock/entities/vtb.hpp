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
  VbkPopTx transaction;
  VbkMerklePath merklePath;
  VbkBlock containingBlock;
  std::vector<VbkBlock> context;

  VTB(VbkPopTx _transaction,
      VbkMerklePath _merklePath,
      VbkBlock _containingBlock,
      std::vector<VbkBlock> _context)
      : transaction(std::move(_transaction)),
        merklePath(std::move(_merklePath)),
        containingBlock(std::move(_containingBlock)),
        context(std::move(_context)) {}

  static VTB fromVbkEncoding(ReadStream& stream) {
    VbkPopTx transaction = VbkPopTx::fromVbkEncoding(stream);
    VbkMerklePath merklePath = VbkMerklePath::fromRaw(stream);
    VbkBlock containingBlock = VbkBlock::fromVbkEncoding(stream);
    auto context = readArrayOf<VbkBlock>(
        stream, 0, MAX_CONTEXT_COUNT, [](ReadStream& stream) {
          return VbkBlock::fromVbkEncoding(stream);
        });

    return VTB(transaction,
               merklePath,
               containingBlock,
               context);
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VTB_HPP_

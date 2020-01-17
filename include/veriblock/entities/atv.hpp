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
  VbkTx transaction;
  VbkMerklePath merklePath;
  VbkBlock containingBlock;
  std::vector<VbkBlock> context;

  ATV(VbkTx _transaction,
      VbkMerklePath _merklePath,
      VbkBlock _containingBlock,
      std::vector<VbkBlock> _context)
      : transaction(std::move(_transaction)),
        merklePath(std::move(_merklePath)),
        containingBlock(std::move(_containingBlock)),
        context(std::move(_context)) {}

  static ATV fromVbkEncoding(ReadStream& stream) {
    VbkTx transaction = VbkTx::fromVbkEncoding(stream);
    VbkMerklePath merklePath = VbkMerklePath::fromRaw(stream);
    VbkBlock containingBlock = VbkBlock::fromVbkEncoding(stream);
    auto context = readArrayOf<VbkBlock>(
        stream, 0, MAX_CONTEXT_COUNT_ALT_PUBLICATION, [](ReadStream& stream) {
          return VbkBlock::fromVbkEncoding(stream);
        });

    return ATV(transaction, merklePath, containingBlock, context);
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_

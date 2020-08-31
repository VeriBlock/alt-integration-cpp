#include "veriblock/blockchain/mempool_block_tree.hpp"

namespace altintegration {

const VbkBlock* MempoolBlockTree::getVbkBlock(
    const typename VbkBlock::hash_t& hash) {
  const VbkBlock* blk_ptr = nullptr;

  blk_ptr = &_tree.vbk().getBlockIndex(hash)->getHeader();
  if (blk_ptr != nullptr) {
    return blk_ptr;
  }

  auto id = hash.trimLE<VbkBlock::id_t::size()>();
  blk_ptr = relations.find(id)->second->header.get();

  return blk_ptr;
}

}  // namespace altintegration
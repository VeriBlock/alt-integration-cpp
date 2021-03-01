// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_FACADE_VBK_BLOCK_INDEX_HPP
#define VERIBLOCK_POP_CPP_FACADE_VBK_BLOCK_INDEX_HPP

#include <veriblock/pop/entities/vbkblock.hpp>

#include "block_index_facade.hpp"

namespace altintegration {

struct VbkBlockIndexFacade final : public BlockIndexFacade<VbkBlock> {
  using base = BlockIndexFacade<VbkBlock>;
  using impl = BlockIndex<VbkBlock>;
  using hash_t = typename impl::hash_t;
  using index_t = VbkBlockIndexFacade;

  VbkBlockIndexFacade(const impl& ptr);

  const ArithUint256& getChainwork() const { return ptr_->chainWork; }

  const std::vector<const AltEndorsement*>& getBlockOfProofEndorsements() const;

  const std::vector<const VbkEndorsement*> getContainingEndorsements() const;

  const std::vector<const VbkEndorsement*> getEndorsedBy() const;

  uint32_t getRefCount() const;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_FACADE_VBK_BLOCK_INDEX_HPP

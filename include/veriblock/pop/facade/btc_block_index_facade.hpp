// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_FACADE_BTC_BLOCK_INDEX_HPP
#define VERIBLOCK_POP_CPP_FACADE_BTC_BLOCK_INDEX_HPP

#include <veriblock/pop/entities/btcblock.hpp>

#include "block_index_facade.hpp"

namespace altintegration {

struct BtcBlockIndexFacade final : public BlockIndexFacade<BtcBlock> {
  using base = BlockIndexFacade<BtcBlock>;
  using impl = BlockIndex<BtcBlock>;
  using hash_t = typename impl::hash_t;

  using index_t = BtcBlockIndexFacade;

  BtcBlockIndexFacade(const impl& ptr);

  const ArithUint256& getChainwork() const { return ptr_->chainWork; }

  const std::vector<const VbkEndorsement*>& getBlockOfProofEndorsements() const;

  const std::vector<int>& getRefs() const noexcept;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_FACADE_BTC_BLOCK_INDEX_HPP

// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BTC_BLOCK_TREE_FACADE_HPP
#define VERIBLOCK_POP_CPP_BTC_BLOCK_TREE_FACADE_HPP

#include <veriblock/pop/blockchain/pop/vbk_block_tree.hpp>

#include "btc_block_index_facade.hpp"
#include "chain_facade.hpp"

namespace altintegration {

struct BtcBlockTreeFacade {
  using hash_t = BtcBlockTree::hash_t;

  explicit BtcBlockTreeFacade(BtcBlockTree& tree) : btc_(&tree) {}

  bool acceptBlockHeader(std::shared_ptr<BtcBlock> header,
                         ValidationState& state);

  const BtcChainParams& getParams() const { return btc_->getParams(); }

  const BtcChainFacade getBestChain() const { return {btc_->getBestChain()}; }

  const std::shared_ptr<BtcBlockIndexFacade> getBlockIndex(
      const hash_t& hash) const {
    auto p = btc_->getBlockIndex(hash);
    return p == nullptr ? nullptr : std::make_shared<BtcBlockIndexFacade>(*p);
  }

 private:
  BtcBlockTree* btc_ = nullptr;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_BTC_BLOCK_TREE_FACADE_HPP

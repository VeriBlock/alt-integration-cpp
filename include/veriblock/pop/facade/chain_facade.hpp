// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_CHAIN_FACADE_HPP
#define VERIBLOCK_POP_CPP_CHAIN_FACADE_HPP

#include <veriblock/pop/blockchain/chain.hpp>

#include "btc_block_index_facade.hpp"
#include "vbk_block_index_facade.hpp"
#include "alt_block_index_facade.hpp"

namespace altintegration {

template <typename BlockIndexFacadeT>
struct ChainFacade {
  using index_t = BlockIndexFacadeT;
  using height_t = int32_t;

  using index_t_impl = typename index_t::impl;
  using chain_t = Chain<index_t_impl>;

  ChainFacade(const Chain<index_t_impl>& chain) : chain_(&chain) {}

  const index_t* operator[](height_t height) const {
    return chain_->operator[](height);
  }

  const index_t* next(const index_t* index) const {
    return chain_->next(index);
  }

  height_t chainHeight() const { return chain_->chainHeight(); }

  bool empty() const { return chain_->empty(); }

  size_t blocksCount() const { return chain_->blocksCount(); }

  const index_t* tip() const { return chain_->tip(); }

  const index_t* first() const { return chain_->first(); }

  const index_t* findFork(const index_t* pindex) const {
    return chain_->findFork(pindex);
  }

  typename chain_t::reverse_iterator_t rbegin() { return chain_->rbegin(); }
  typename chain_t::iterator_t begin() { return chain_->begin(); }
  typename chain_t::const_iterator_t begin() const { return chain_->begin(); }
  typename chain_t::reverse_iterator_t rend() { return chain_->rend(); }
  typename chain_t::iterator_t end() { return chain_->end(); }
  typename chain_t::const_iterator_t end() const { return chain_->end(); }

 private:
  const Chain<index_t_impl>* chain_ = nullptr;
};

using AltChainFacade = ChainFacade<AltBlockIndexFacade>;
using VbkChainFacade = ChainFacade<VbkBlockIndexFacade>;
using BtcChainFacade = ChainFacade<BtcBlockIndexFacade>;

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_CHAIN_FACADE_HPP

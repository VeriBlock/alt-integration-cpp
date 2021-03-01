// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BLOCK_INDEX_FACADE_HPP
#define VERIBLOCK_POP_CPP_BLOCK_INDEX_FACADE_HPP

#include <veriblock/pop/blockchain/block_index.hpp>

namespace altintegration {

template <typename Block>
struct BlockIndexFacade {
  using impl = BlockIndex<Block>;
  using hash_t = typename impl::hash_t;
  using block_t = typename impl::block_t;

  using index_t = BlockIndexFacade<Block>;

  explicit BlockIndexFacade(const impl& ptr) : ptr_(&ptr) {}

  const index_t* getPrevious() const {
    const auto* prev = ptr_->getPrev();
    return prev ? prev : nullptr;
  }

  bool isConnected() const noexcept { return ptr_->isConnected(); }

  uint32_t getStatus() const noexcept { return ptr_->getStatus(); }

  bool isValidTip() const { return ptr_->isValidTip(); }

  const block_t& getHeader() const noexcept { return ptr_->getHeader(); }

  const hash_t& getHash() const { return ptr_->getHash(); }

  const index_t* getAncestor(int height) const {
    return ptr_->getAncestor(height);
  }

 protected:
  const impl* ptr_ = nullptr;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_BLOCK_INDEX_FACADE_HPP

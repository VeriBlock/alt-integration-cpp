// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCKS_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCKS_STORAGE_HPP_

#include <veriblock/storage/block_repository_inmem.hpp>

namespace altintegration {

template <typename StoredBlock>
class BlocksStorage {
  using height_t = typename StoredBlock::height_t;
  using hash_t = typename StoredBlock::hash_t;
 public:
  virtual ~BlocksStorage() = default;

  BlocksStorage()
      : brepo_(
            std::move(std::make_shared<BlockRepositoryInmem<StoredBlock>>())) {}

  BlockRepository<StoredBlock>& blocks() { return *brepo_; }
  const BlockRepository<StoredBlock>& blocks() const {
    return *brepo_; }

  void saveTip(const StoredBlock& tip) {
    tipHeight_ = tip.height;
    tipHash_ = tip.getHash();
  }

  std::pair<height_t, hash_t> loadTip() const {
    return std::pair<height_t, hash_t>{tipHeight_, tipHash_};
  }

 protected:
  std::shared_ptr<BlockRepository<StoredBlock>> brepo_;
  height_t tipHeight_;
  hash_t tipHash_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCKS_STORAGE_HPP_
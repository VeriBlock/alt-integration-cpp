// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_TIPS_REPOSITORY_INMEM_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_TIPS_REPOSITORY_INMEM_HPP_

#include <veriblock/storage/tips_repository.hpp>

namespace altintegration {

template <typename Block>
struct TipsRepositoryInmem : public TipsRepository<Block> {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;

  ~TipsRepositoryInmem() override = default;

  bool get(std::pair<typename Block::height_t, typename Block::hash_t>* out)
      const override {
    if (out) {
      *out = {_height, _hash};
    }
    return true;
  }

  bool put(const stored_block_t& tip) override {
    _hash = tip.getHash();
    _height = tip.height;
    return true;
  }

 private:
  hash_t _hash;
  height_t _height;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_INMEM_TIPS_REPOSITORY_INMEM_HPP_

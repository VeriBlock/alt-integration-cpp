// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_TIPS_REPOSITORY_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_TIPS_REPOSITORY_HPP_

#include <cstdint>
#include <memory>
#include <vector>
#include <veriblock/slice.hpp>
#include <veriblock/storage/cursor.hpp>

namespace altintegration {

/**
 * @class TipsRepository
 *
 * @brief Represents a blockchain tip stored on disk.
 *
 * @tparam Block Stored Block type
 *
 */
template <typename Block>
struct TipsRepository {
  //! stored block type
  using stored_block_t = Block;
  //! block hash type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;

  virtual ~TipsRepository() = default;

  /**
   * Load a tip hash and height from disk in memory.
   * @param outHash[out] if non-null, hash will be written here. If null
   * passed, outHash argument is ignored.
   * @param outHeight[out] if non-null, height will be written here. If null
   * passed, outHeight argument is ignored.
   * @return true if tip found, false otherwise.
   */
  virtual bool get(std::pair<typename Block::height_t, typename Block::hash_t>*
                       out) const = 0;

  /**
   * Write a tip. If tip already exists, db will overwrite it.
   * @param tip to be written
   * @return true if tip already existed in db and we overwrote it. False
   * otherwise.
   */
  virtual bool put(const stored_block_t& tip) = 0;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_TIPS_REPOSITORY_HPP_

// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BLOCK_PROVIDER_HPP
#define VERIBLOCK_POP_CPP_BLOCK_PROVIDER_HPP

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/validation_state.hpp>

/**
 * @defgroup interfaces Interfaces to be implemented
 * These interfaces must be implemented by Altchain developers for integration
 * of POP protocol.
 */

namespace altintegration {

/**
 * @struct BlockProvider
 *
 * An abstraction over on-disk storage iterator.
 *
 * veriblock-pop-cpp does not dictate how to iterate over the storage. Altchain
 * must create derived class.
 *
 * @ingroup interfaces
 */
struct BlockProvider {
  virtual ~BlockProvider() = default;

  virtual bool getTip(BlockIndex<BtcBlock>& out) const = 0;
  virtual bool getTip(BlockIndex<VbkBlock>& out) const = 0;
  virtual bool getTip(BlockIndex<AltBlock>& out) const = 0;

  virtual bool getBlock(const typename BtcBlock::hash_t& hash,
                        BlockIndex<BtcBlock>& out) const = 0;
  virtual bool getBlock(const typename VbkBlock::hash_t& hash,
                        BlockIndex<VbkBlock>& out) const = 0;
  virtual bool getBlock(const typename AltBlock::hash_t& hash,
                        BlockIndex<AltBlock>& out) const = 0;
};

}  // namespace altintegration

#endif
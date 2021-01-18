// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BLOCK_PROVIDER_HPP
#define VERIBLOCK_POP_CPP_BLOCK_PROVIDER_HPP

#include <memory>

#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/storage/block_iterator.hpp"
#include "veriblock/validation_state.hpp"

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

namespace details {

template <typename BlockT>
struct GenericBlockWriter {
  virtual ~GenericBlockWriter() = default;

  virtual bool writeBlock(const BlockIndex<BlockT>& value) = 0;

  virtual bool writeTip(const BlockIndex<BlockT>& value) = 0;
};

template <typename BlockT>
struct GenericBlockReader {
  using hash_t = typename BlockT::hash_t;

  virtual ~GenericBlockReader() = default;

  virtual bool getTipHash(hash_t& out) const = 0;

  virtual bool getBlock(const hash_t& hash, BlockIndex<BlockT>& out) const = 0;

  virtual std::shared_ptr<BlockIterator<BlockT>> getBlockIterator() const = 0;
};

}  // namespace details

struct BlockProvider {
  virtual ~BlockProvider() = default;

  virtual std::shared_ptr<details::GenericBlockReader<AltBlock>>
  getAltBlockReader() const = 0;

  virtual std::shared_ptr<details::GenericBlockReader<VbkBlock>>
  getVbkBlockReader() const = 0;

  virtual std::shared_ptr<details::GenericBlockReader<BtcBlock>>
  getBtcBlockReader() const = 0;

  virtual std::shared_ptr<details::GenericBlockWriter<AltBlock>>
  getAltBlockWriter() const = 0;

  virtual std::shared_ptr<details::GenericBlockWriter<VbkBlock>>
  getVbkBlockWriter() const = 0;

  virtual std::shared_ptr<details::GenericBlockWriter<BtcBlock>>
  getBtcBlockWriter() const = 0;
};

}  // namespace altintegration

#endif
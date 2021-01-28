// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BLOCK_READER_HPP
#define VERIBLOCK_POP_CPP_BLOCK_READER_HPP

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
 * @ingroup interfaces
 */

struct BlockReader {
  virtual ~BlockReader() = default;

  virtual bool getAltTip(AltBlock::hash_t& out) const = 0;
  virtual bool getVbkTip(VbkBlock::hash_t& out) const = 0;
  virtual bool getBtcTip(BtcBlock::hash_t& out) const = 0;

  // clang-format off
  virtual std::shared_ptr<BlockIterator<AltBlock>> getAltBlockIterator() const = 0;
  virtual std::shared_ptr<BlockIterator<VbkBlock>> getVbkBlockIterator() const = 0;
  virtual std::shared_ptr<BlockIterator<BtcBlock>> getBtcBlockIterator() const = 0;
  // clang-format on

  template <typename Block>
  std::shared_ptr<BlockIterator<Block>> getBlockIterator() const;

  template <typename Block>
  bool getTip(typename Block::hash_t& out) const;
};

// clang-format off
template <>
inline std::shared_ptr<BlockIterator<AltBlock>> BlockReader::getBlockIterator() const {
  return this->getAltBlockIterator();
}

template <>
inline std::shared_ptr<BlockIterator<VbkBlock>> BlockReader::getBlockIterator() const {
  return this->getVbkBlockIterator();
}

template <>
inline std::shared_ptr<BlockIterator<BtcBlock>> BlockReader::getBlockIterator() const {
  return this->getBtcBlockIterator();
}
// clang-format on

template <>
inline bool BlockReader::getTip<AltBlock>(AltBlock::hash_t& out) const {
  return this->getAltTip(out);
}

template <>
inline bool BlockReader::getTip<VbkBlock>(VbkBlock::hash_t& out) const {
  return this->getVbkTip(out);
}

template <>
inline bool BlockReader::getTip<BtcBlock>(BtcBlock::hash_t& out) const {
  return this->getBtcTip(out);
}

}  // namespace altintegration

#endif
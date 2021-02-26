// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BLOCK_BATCH_HPP
#define VERIBLOCK_POP_CPP_BLOCK_BATCH_HPP

#include <veriblock/pop/blockchain/block_index.hpp>
#include <veriblock/pop/entities/altblock.hpp>
#include <veriblock/pop/entities/btcblock.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>

namespace altintegration {

struct BlockBatch {
  virtual ~BlockBatch() = default;

  virtual void writeBlock(const BlockIndex<AltBlock>& value) = 0;
  virtual void writeBlock(const BlockIndex<VbkBlock>& value) = 0;
  virtual void writeBlock(const BlockIndex<BtcBlock>& value) = 0;

  virtual void writeTip(const BlockIndex<AltBlock>& value) = 0;
  virtual void writeTip(const BlockIndex<VbkBlock>& value) = 0;
  virtual void writeTip(const BlockIndex<BtcBlock>& value) = 0;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_BLOCK_BATCH_HPP

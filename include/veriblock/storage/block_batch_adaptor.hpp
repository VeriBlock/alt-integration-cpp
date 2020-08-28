// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_WRITE_BATCH_ADAPTOR_HPP
#define VERIBLOCK_POP_CPP_WRITE_BATCH_ADAPTOR_HPP

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/vbkblock.hpp>

namespace altintegration {

struct BlockBatchAdaptor {
  virtual ~BlockBatchAdaptor() = default;

  virtual bool writeBlock(const BlockIndex<BtcBlock>& value) = 0;
  virtual bool writeBlock(const BlockIndex<VbkBlock>& value) = 0;
  virtual bool writeBlock(const BlockIndex<AltBlock>& value) = 0;

  virtual bool writeTip(const BlockIndex<BtcBlock>& value) = 0;
  virtual bool writeTip(const BlockIndex<VbkBlock>& value) = 0;
  virtual bool writeTip(const BlockIndex<AltBlock>& value) = 0;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_WRITE_BATCH_ADAPTOR_HPP

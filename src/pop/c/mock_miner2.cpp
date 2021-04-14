// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "entities/btcblock.hpp"
#include "entities/vbkblock.hpp"
#include "mock_miner2.hpp"
#include "veriblock/pop/assert.hpp"

POP_ENTITY_FREE_SIGNATURE(mock_miner) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_NEW_SIGNATURE(mock_miner) { return new POP_ENTITY_NAME(mock_miner); }

POP_ENTITY_FUNCTION_SIGNATURE(mock_miner,
                              POP_ENTITY_NAME(btc_block) *,
                              mineBtcBlock,
                              const POP_ENTITY_NAME(btc_block) * tip) {
  VBK_ASSERT(self);
  VBK_ASSERT(tip);

  auto* block = self->ref.btc().getBlockIndex(tip->ref.getHash());
  if (block == nullptr) {
    return nullptr;
  }

  auto* new_block = self->ref.mineBtcBlocks(1, *block);
  VBK_ASSERT(new_block);

  auto* res = new POP_ENTITY_NAME(btc_block);
  res->ref = new_block->getHeader();

  return res;
}

POP_ENTITY_FUNCTION_SIGNATURE(mock_miner,
                              POP_ENTITY_NAME(btc_block) *,
                              mineBtcBlockTip) {
  VBK_ASSERT(self);

  auto* new_block = self->ref.mineBtcBlocks(1);
  VBK_ASSERT(new_block);

  auto* res = new POP_ENTITY_NAME(btc_block);
  res->ref = new_block->getHeader();

  return res;
}

POP_ENTITY_FUNCTION_SIGNATURE(mock_miner,
                              POP_ENTITY_NAME(vbk_block) *,
                              mineBtcBlock,
                              const POP_ENTITY_NAME(vbk_block) * tip) {
  VBK_ASSERT(self);
  VBK_ASSERT(tip);

  auto* block = self->ref.vbk().getBlockIndex(tip->ref.getHash());
  if (block == nullptr) {
    return nullptr;
  }

  auto* new_block = self->ref.mineVbkBlocks(1, *block);
  VBK_ASSERT(new_block);

  auto* res = new POP_ENTITY_NAME(vbk_block);
  res->ref = new_block->getHeader();

  return res;
}

POP_ENTITY_FUNCTION_SIGNATURE(mock_miner,
                              POP_ENTITY_NAME(vbk_block) *,
                              mineVbkBlockTip) {
  VBK_ASSERT(self);

  auto* new_block = self->ref.mineVbkBlocks(1);
  VBK_ASSERT(new_block);

  auto* res = new POP_ENTITY_NAME(vbk_block);
  res->ref = new_block->getHeader();

  return res;
}
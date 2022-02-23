// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "entities/atv.hpp"
#include "entities/btcblock.hpp"
#include "entities/publication_data.hpp"
#include "entities/vbkblock.hpp"
#include "entities/vtb.hpp"
#include "mock_miner.hpp"
#include "veriblock/pop/assert.hpp"

POP_ENTITY_FREE_SIGNATURE(mock_miner) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_NEW_FUNCTION(mock_miner) {
  return new POP_ENTITY_NAME(mock_miner);
}

POP_ENTITY_CUSTOM_FUNCTION(mock_miner,
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

POP_ENTITY_CUSTOM_FUNCTION(mock_miner,
                           POP_ENTITY_NAME(btc_block) *,
                           mineBtcBlockTip) {
  VBK_ASSERT(self);

  auto* new_block = self->ref.mineBtcBlocks(1);
  VBK_ASSERT(new_block);

  auto* res = new POP_ENTITY_NAME(btc_block);
  res->ref = new_block->getHeader();

  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(mock_miner,
                           POP_ENTITY_NAME(vbk_block) *,
                           mineVbkBlock,
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

POP_ENTITY_CUSTOM_FUNCTION(mock_miner,
                           POP_ENTITY_NAME(vbk_block) *,
                           mineVbkBlockTip) {
  VBK_ASSERT(self);

  auto* new_block = self->ref.mineVbkBlocks(1);
  VBK_ASSERT(new_block);

  auto* res = new POP_ENTITY_NAME(vbk_block);
  res->ref = new_block->getHeader();

  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(mock_miner,
                           POP_ENTITY_NAME(atv) *,
                           mineATV,
                           const POP_ENTITY_NAME(publication_data) * pub_data) {
  VBK_ASSERT(self);
  VBK_ASSERT(pub_data);

  auto vbktx = self->ref.createVbkTxEndorsingAltBlock(pub_data->ref);
  auto* block = self->ref.mineVbkBlocks(1, {vbktx});

  auto* res = new POP_ENTITY_NAME(atv);
  res->ref = self->ref.createATV(block->getHeader(), vbktx);

  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(mock_miner,
                           POP_ENTITY_NAME(vtb) *,
                           mineVTB,
                           const POP_ENTITY_NAME(vbk_block) * endorsed_block,
                           const POP_ENTITY_NAME(btc_block) *
                               last_known_btc_block) {
  VBK_ASSERT(self);
  VBK_ASSERT(endorsed_block);
  VBK_ASSERT(last_known_btc_block);

  auto* res = new POP_ENTITY_NAME(vtb);
  res->ref = self->ref.endorseVbkBlock(endorsed_block->ref,
                                       last_known_btc_block->ref.getHash());

  return res;
}
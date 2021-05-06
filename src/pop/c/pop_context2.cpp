// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "adaptors/logger.hpp"
#include "adaptors/payloads_provider_impl.hpp"
#include "config2.hpp"
#include "entities/altblock.hpp"
#include "entities/block_index.hpp"
#include "pop_context2.hpp"
#include "storage2.hpp"
#include "validation_state2.hpp"
#include "veriblock/pop/assert.hpp"

POP_ENTITY_FREE_SIGNATURE(pop_context) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_NEW_FUNCTION(pop_context,
                        const POP_ENTITY_NAME(config) * config,
                        const POP_ENTITY_NAME(storage) * storage,
                        POP_ARRAY_NAME(string) log_lvl) {
  VBK_ASSERT(config);
  VBK_ASSERT(storage);
  VBK_ASSERT(log_lvl.data);
  VBK_ASSERT(storage->ref);
  VBK_ASSERT(config->ref);
  VBK_ASSERT(config->ref->alt);

  // set logger
  altintegration::SetLogger<adaptors::Logger2>(altintegration::StringToLevel(
      std::string(log_lvl.data, log_lvl.data + log_lvl.size)));

  auto* res = new POP_ENTITY_NAME(pop_context);

  res->storage = storage->ref;
  res->payloads_storage =
      std::make_shared<adaptors::PayloadsStorageImpl>(*storage->ref);
  res->ref =
      altintegration::PopContext::create(config->ref, res->payloads_storage);

  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           bool,
                           accept_block_header,
                           const POP_ENTITY_NAME(alt_block) * block,
                           POP_ENTITY_NAME(validation_state) * state) {
  VBK_ASSERT(self);
  VBK_ASSERT(block);
  VBK_ASSERT(state);

  return self->ref->getAltBlockTree().acceptBlockHeader(block->ref, state->ref);
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(alt_block_index) *,
                           alt_get_block_index,
                           POP_ARRAY_NAME(u8) hash) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash.data);

  auto* index = self->ref->getAltBlockTree().getBlockIndex(
      std::vector<uint8_t>(hash.data, hash.data + hash.size));
  if (index == nullptr) {
    return nullptr;
  }
  return new POP_ENTITY_NAME(alt_block_index){*index};
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(vbk_block_index) *,
                           vbk_get_block_index,
                           POP_ARRAY_NAME(u8) hash) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash.data);

  altintegration::VbkBlock::hash_t tmp_hash(
      altintegration::Slice<const uint8_t>(hash.data, hash.size));
  auto* index = self->ref->getVbkBlockTree().getBlockIndex(tmp_hash);
  if (index == nullptr) {
    return nullptr;
  }
  return new POP_ENTITY_NAME(vbk_block_index){*index};
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(btc_block_index) *,
                           btc_get_block_index,
                           POP_ARRAY_NAME(u8) hash) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash.data);

  altintegration::BtcBlock::hash_t s_hash(
      altintegration::Slice<const uint8_t>(hash.data, hash.size));
  auto* index = self->ref->getBtcBlockTree().getBlockIndex(s_hash);
  if (index == nullptr) {
    return nullptr;
  }
  return new POP_ENTITY_NAME(btc_block_index){*index};
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(alt_block_index) *,
                           alt_get_best_block) {
  VBK_ASSERT(self);

  auto* tip = self->ref->getAltBlockTree().getBestChain().tip();
  VBK_ASSERT(tip);
  return new POP_ENTITY_NAME(alt_block_index){*tip};
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(vbk_block_index) *,
                           vbk_get_best_block) {
  VBK_ASSERT(self);

  auto* tip = self->ref->getVbkBlockTree().getBestChain().tip();
  VBK_ASSERT(tip);
  return new POP_ENTITY_NAME(vbk_block_index){*tip};
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(btc_block_index) *,
                           btc_get_best_block) {
  VBK_ASSERT(self);

  auto* tip = self->ref->getBtcBlockTree().getBestChain().tip();
  VBK_ASSERT(tip);
  return new POP_ENTITY_NAME(btc_block_index){*tip};
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(alt_block_index) *,
                           alt_get_bootstrap_block) {
  VBK_ASSERT(self);

  auto* first = self->ref->getAltBlockTree().getBestChain().first();
  VBK_ASSERT(first);
  return new POP_ENTITY_NAME(alt_block_index){*first};
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(vbk_block_index) *,
                           vbk_get_bootstrap_block) {
  VBK_ASSERT(self);

  auto* first = self->ref->getVbkBlockTree().getBestChain().first();
  VBK_ASSERT(first);
  return new POP_ENTITY_NAME(vbk_block_index){*first};
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(btc_block_index) *,
                           btc_get_bootstrap_block) {
  VBK_ASSERT(self);

  auto* first = self->ref->getBtcBlockTree().getBestChain().first();
  VBK_ASSERT(first);
  return new POP_ENTITY_NAME(btc_block_index){*first};
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(alt_block_index) *,
                           alt_get_block_at_active_chain,
                           uint32_t height) {
  VBK_ASSERT(self);

  auto* index = self->ref->getAltBlockTree().getBestChain()[height];
  if (index == nullptr) {
    return nullptr;
  }
  return new POP_ENTITY_NAME(alt_block_index){*index};
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(vbk_block_index) *,
                           vbk_get_block_at_active_chain,
                           uint32_t height) {
  VBK_ASSERT(self);

  auto* index = self->ref->getVbkBlockTree().getBestChain()[height];
  if (index == nullptr) {
    return nullptr;
  }
  return new POP_ENTITY_NAME(vbk_block_index){*index};
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(btc_block_index) *,
                           btc_get_block_at_active_chain,
                           uint32_t height) {
  VBK_ASSERT(self);

  auto* index = self->ref->getBtcBlockTree().getBestChain()[height];
  if (index == nullptr) {
    return nullptr;
  }
  return new POP_ENTITY_NAME(btc_block_index){*index};
}

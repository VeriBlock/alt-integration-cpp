// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "adaptors/logger.hpp"
#include "config.hpp"
#include "entities/altblock.hpp"
#include "entities/block_index.hpp"
#include "entities/pop_payouts.hpp"
#include "entities/popdata.hpp"
#include "pop_context.hpp"
#include "storage.hpp"
#include "validation_state.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/storage/adaptors/payloads_provider_impl.hpp"

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
  altintegration::SetLogger<adaptors::Logger>(altintegration::StringToLevel(
      std::string(log_lvl.data, log_lvl.data + log_lvl.size)));

  auto* res = new POP_ENTITY_NAME(pop_context);
  res->storage = storage->ref;
  res->ref = altintegration::PopContext::create(
      config->ref,
      std::make_shared<altintegration::adaptors::PayloadsStorageImpl>(
          *storage->ref),
      std::make_shared<altintegration::adaptors::BlockReaderImpl>(
          *storage->ref));

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
                           void,
                           accept_block,
                           POP_ARRAY_NAME(u8) hash,
                           const POP_ENTITY_NAME(pop_data) * pop_data) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash.data);

  if (pop_data == nullptr) {
    self->ref->getAltBlockTree().acceptBlock(
        std::vector<uint8_t>(hash.data, hash.data + hash.size),
        altintegration::PopData());
  } else {
    self->ref->getAltBlockTree().acceptBlock(
        std::vector<uint8_t>(hash.data, hash.data + hash.size), pop_data->ref);
  }
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           bool,
                           set_state,
                           POP_ARRAY_NAME(u8) hash,
                           POP_ENTITY_NAME(validation_state) * state) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash.data);
  VBK_ASSERT(state);

  return self->ref->getAltBlockTree().setState(
      std::vector<uint8_t>(hash.data, hash.data + hash.size), state->ref);
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           int,
                           compare_pop_score,
                           POP_ARRAY_NAME(u8) A_block_hash,
                           POP_ARRAY_NAME(u8) B_block_hash) {
  VBK_ASSERT(self);
  VBK_ASSERT(A_block_hash.data);
  VBK_ASSERT(B_block_hash.data);

  return self->ref->getAltBlockTree().comparePopScore(
      std::vector<uint8_t>(A_block_hash.data,
                           A_block_hash.data + A_block_hash.size),
      std::vector<uint8_t>(B_block_hash.data,
                           B_block_hash.data + B_block_hash.size));
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(pop_payout),
                           get_pop_payouts,
                           POP_ARRAY_NAME(u8) hash,
                           POP_ENTITY_NAME(validation_state) * state) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash.data);
  VBK_ASSERT(state);

  altintegration::PopPayouts payouts{};
  bool ret = self->ref->getPopPayout(
      std::vector<uint8_t>(hash.data, hash.data + hash.size),
      payouts,
      state->ref);

  POP_ARRAY_NAME(pop_payout) res;
  res.size = 0;

  if (!ret) {
    return res;
  }

  res.size = payouts.size();
  res.data = new POP_ENTITY_NAME(pop_payout)*[payouts.size()];
  size_t i = 0;
  for (auto& el : payouts) {
    res.data[i] = new POP_ENTITY_NAME(pop_payout);
    res.data[i]->payout_info = el.first;
    res.data[i++]->amount = el.second;
  }

  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           void,
                           remove_subtree,
                           POP_ARRAY_NAME(u8) hash) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash.data);

  self->ref->getAltBlockTree().removeSubtree(
      std::vector<uint8_t>(hash.data, hash.data + hash.size));
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(alt_block_index) *,
                           alt_get_block_index,
                           POP_ARRAY_NAME(u8) hash,
                           POP_ENTITY_NAME(validation_state) * state) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash.data);
  VBK_ASSERT(state);

  auto* index = self->ref->getAltBlockTree().getBlockIndex(
      std::vector<uint8_t>(hash.data, hash.data + hash.size));
  if (index == nullptr) {
    state->ref.Invalid("Can not find block");
    return nullptr;
  }
  return new POP_ENTITY_NAME(alt_block_index){*index};
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(vbk_block_index) *,
                           vbk_get_block_index,
                           POP_ARRAY_NAME(u8) hash,
                           POP_ENTITY_NAME(validation_state) * state) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash.data);
  VBK_ASSERT(state);

  if (hash.size == altintegration::VbkBlock::hash_t::size()) {
    altintegration::VbkBlock::hash_t tmp_hash(
        altintegration::Slice<const uint8_t>(hash.data, hash.size));
    auto* index = self->ref->getVbkBlockTree().getBlockIndex(tmp_hash);
    if (index == nullptr) {
      state->ref.Invalid("Can not find block");
      return nullptr;
    }
    return new POP_ENTITY_NAME(vbk_block_index){*index};
  }

  if (hash.size == altintegration::VbkBlock::prev_hash_t::size()) {
    altintegration::VbkBlock::prev_hash_t tmp_hash(
        altintegration::Slice<const uint8_t>(hash.data, hash.size));
    auto* index = self->ref->getVbkBlockTree().getBlockIndex(tmp_hash);
    if (index == nullptr) {
      state->ref.Invalid("Can not find block");
      return nullptr;
    }
    return new POP_ENTITY_NAME(vbk_block_index){*index};
  }

  state->ref.Invalid(
      "Wrong hash size",
      altintegration::format("provide hash size: {}, expected size: {} or: {}",
                             hash.size,
                             altintegration::VbkBlock::hash_t::size(),
                             altintegration::VbkBlock::prev_hash_t::size()));
  return nullptr;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(btc_block_index) *,
                           btc_get_block_index,
                           POP_ARRAY_NAME(u8) hash,
                           POP_ENTITY_NAME(validation_state) * state) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash.data);
  VBK_ASSERT(state);

  if (hash.size == altintegration::BtcBlock::hash_t::size()) {
    altintegration::BtcBlock::hash_t tmp_hash(
        altintegration::Slice<const uint8_t>(hash.data, hash.size));
    auto* index = self->ref->getBtcBlockTree().getBlockIndex(tmp_hash);
    if (index == nullptr) {
      state->ref.Invalid("Can not find block");
      return nullptr;
    }
    return new POP_ENTITY_NAME(btc_block_index){*index};
  }

  state->ref.Invalid(
      "Wrong hash size",
      altintegration::format("provide hash size: {}, expected size: {}",
                             hash.size,
                             altintegration::BtcBlock::hash_t::size()));
  return nullptr;
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

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(array_u8),
                           get_payload_containing_blocks,
                           POP_ARRAY_NAME(u8) id) {
  VBK_ASSERT(self);

  auto alt_hashes =
      self->ref->getAltBlockTree().getPayloadsIndex().getContainingAltBlocks(
          std::vector<uint8_t>(id.data, id.data + id.size));

  POP_ARRAY_NAME(array_u8) res;
  res.size = alt_hashes.size();
  res.data = new POP_ARRAY_NAME(u8)[res.size];
  auto it = alt_hashes.begin();
  for (size_t i = 0; i < res.size; ++i, ++it) {
    auto& src = *it;
    auto& dst = res.data[i];
    dst.size = src.size();
    dst.data = new uint8_t[dst.size];
    std::copy(src.begin(), src.end(), dst.data);
  }
  return res;
}

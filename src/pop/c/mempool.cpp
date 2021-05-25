// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "entities/atv.hpp"
#include "entities/popdata.hpp"
#include "entities/vbkblock.hpp"
#include "entities/vtb.hpp"
#include "pop_context2.hpp"
#include "validation_state2.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/c/mempool.h"

static int handleSubmitResponse(altintegration::MemPool::SubmitResult e) {
  using S = altintegration::MemPool::Status;
  switch (e.status) {
    case S::VALID:
      return 0;
    case S::FAILED_STATEFUL:
      return 1;
    case S::FAILED_STATELESS:
      return 2;
  }

  VBK_ASSERT_MSG(false, "Unhandled case");
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           int,
                           mempool_submit_vbk,
                           const POP_ENTITY_NAME(vbk_block) * vbk_block,
                           POP_ENTITY_NAME(validation_state) * state) {
  VBK_ASSERT(self);
  VBK_ASSERT(vbk_block);
  VBK_ASSERT(state);

  auto res = self->ref->getMemPool().submit<altintegration::VbkBlock>(
      vbk_block->ref, state->ref);
  return handleSubmitResponse(res);
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           int,
                           mempool_submit_vtb,
                           const POP_ENTITY_NAME(vtb) * vtb,
                           POP_ENTITY_NAME(validation_state) * state) {
  VBK_ASSERT(self);
  VBK_ASSERT(vtb);
  VBK_ASSERT(state);

  auto res =
      self->ref->getMemPool().submit<altintegration::VTB>(vtb->ref, state->ref);
  return handleSubmitResponse(res);
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           int,
                           mempool_submit_atv,
                           const POP_ENTITY_NAME(atv) * atv,
                           POP_ENTITY_NAME(validation_state) * state) {
  VBK_ASSERT(self);
  VBK_ASSERT(atv);
  VBK_ASSERT(state);

  auto res =
      self->ref->getMemPool().submit<altintegration::ATV>(atv->ref, state->ref);
  return handleSubmitResponse(res);
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(pop_data) *,
                           mempool_generate_pop_data) {
  VBK_ASSERT(self);

  auto* res = new POP_ENTITY_NAME(pop_data);
  res->ref = self->ref->getMemPool().generatePopData();
  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(atv) *,
                           mempool_get_atv,
                           POP_ARRAY_NAME(u8) id) {
  VBK_ASSERT(self);

  auto atv_id = altintegration::ATV::id_t(
      altintegration::Slice<const uint8_t>(id.data, id.size));
  auto atv = self->ref->getMemPool().get<altintegration::ATV>(atv_id);

  auto res = new POP_ENTITY_NAME(atv);
  res->ref = *atv;
  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(vtb) *,
                           mempool_get_vtb,
                           POP_ARRAY_NAME(u8) id) {
  VBK_ASSERT(self);

  auto vtb_id = altintegration::VTB::id_t(
      altintegration::Slice<const uint8_t>(id.data, id.size));
  auto vtb = self->ref->getMemPool().get<altintegration::VTB>(vtb_id);

  auto res = new POP_ENTITY_NAME(vtb);
  res->ref = *vtb;
  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(vbk_block) *,
                           mempool_get_vbk_block,
                           POP_ARRAY_NAME(u8) id) {
  VBK_ASSERT(self);

  auto vbk_id = altintegration::VbkBlock::id_t(
      altintegration::Slice<const uint8_t>(id.data, id.size));
  auto vbk = self->ref->getMemPool().get<altintegration::VbkBlock>(vbk_id);

  auto res = new POP_ENTITY_NAME(vbk_block);
  res->ref = *vbk;
  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context, POP_ARRAY_NAME(atv), mempool_get_atvs) {
  VBK_ASSERT(self);

  auto& atvs = self->ref->getMemPool().getMap<altintegration::ATV>();

  POP_ARRAY_NAME(atv) res;
  res.size = atvs.size();
  res.data = new POP_ENTITY_NAME(atv)*[atvs.size()];
  size_t i = 0;
  for (const auto& it : atvs) {
    res.data[i] = new POP_ENTITY_NAME(atv);
    res.data[i++]->ref = *it.second;
  }
  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context, POP_ARRAY_NAME(vtb), mempool_get_vtbs) {
  VBK_ASSERT(self);

  auto& vtbs = self->ref->getMemPool().getMap<altintegration::VTB>();

  POP_ARRAY_NAME(vtb) res;
  res.size = vtbs.size();
  res.data = new POP_ENTITY_NAME(vtb)*[vtbs.size()];
  size_t i = 0;
  for (const auto& it : vtbs) {
    res.data[i] = new POP_ENTITY_NAME(vtb);
    res.data[i++]->ref = *it.second;
  }
  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(vbk_block),
                           mempool_get_vbk_blocks) {
  VBK_ASSERT(self);

  auto& vbks = self->ref->getMemPool().getMap<altintegration::VbkBlock>();

  POP_ARRAY_NAME(vbk_block) res;
  res.size = vbks.size();
  res.data = new POP_ENTITY_NAME(vbk_block)*[vbks.size()];
  size_t i = 0;
  for (const auto& it : vbks) {
    res.data[i] = new POP_ENTITY_NAME(vbk_block);
    res.data[i++]->ref = *it.second;
  }
  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(atv),
                           mempool_get_in_flight_atvs) {
  VBK_ASSERT(self);

  auto& atvs = self->ref->getMemPool().getInFlightMap<altintegration::ATV>();

  POP_ARRAY_NAME(atv) res;
  res.size = atvs.size();
  res.data = new POP_ENTITY_NAME(atv)*[atvs.size()];
  size_t i = 0;
  for (const auto& it : atvs) {
    res.data[i] = new POP_ENTITY_NAME(atv);
    res.data[i++]->ref = *it.second;
  }
  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(vtb),
                           mempool_get_in_flight_vtbs) {
  VBK_ASSERT(self);

  auto& vtbs = self->ref->getMemPool().getInFlightMap<altintegration::VTB>();

  POP_ARRAY_NAME(vtb) res;
  res.size = vtbs.size();
  res.data = new POP_ENTITY_NAME(vtb)*[vtbs.size()];
  size_t i = 0;
  for (const auto& it : vtbs) {
    res.data[i] = new POP_ENTITY_NAME(vtb);
    res.data[i++]->ref = *it.second;
  }
  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(vbk_block),
                           mempool_get_in_flight_vbk_blocks) {
  VBK_ASSERT(self);

  auto& vbks =
      self->ref->getMemPool().getInFlightMap<altintegration::VbkBlock>();

  POP_ARRAY_NAME(vbk_block) res;
  res.size = vbks.size();
  res.data = new POP_ENTITY_NAME(vbk_block)*[vbks.size()];
  size_t i = 0;
  for (const auto& it : vbks) {
    res.data[i] = new POP_ENTITY_NAME(vbk_block);
    res.data[i++]->ref = *it.second;
  }
  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(array_u8),
                           mempool_get_missing_btc_blocks) {
  VBK_ASSERT(self);

  auto missing_hashes = self->ref->getMemPool().getMissingBtcBlocks();

  POP_ARRAY_NAME(array_u8) res;
  res.size = missing_hashes.size();
  res.data = new POP_ARRAY_NAME(u8)[res.size];
  for (size_t i = 0; i < res.size; ++i) {
    auto& src = missing_hashes[i];
    auto& dst = res.data[i];
    dst.size = src.size();
    dst.data = new uint8_t[dst.size];
    std::copy(src.begin(), src.end(), dst.data);
  }

  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           void,
                           mempool_remove_all,
                           const POP_ENTITY_NAME(pop_data) * pop_data) {
  VBK_ASSERT(self);
  VBK_ASSERT(pop_data);

  self->ref->getMemPool().removeAll(pop_data->ref);
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context, void, mempool_clean_up) {
  VBK_ASSERT(self);

  self->ref->getMemPool().cleanUp();
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context, void, mempool_clear) {
  VBK_ASSERT(self);

  self->ref->getMemPool().clear();
}

// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "adaptors/logger.hpp"
#include "adaptors/payloads_provider_impl.hpp"
#include "config2.hpp"
#include "pop_context2.hpp"
#include "storage2.hpp"
#include "veriblock/pop/assert.hpp"
#include "entities/vbkblock.hpp"
#include "entities/vtb.hpp"
#include "entities/atv.hpp"
#include "entities/altblock.hpp"
#include "validation_state2.hpp"

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
                           submit_vbk,
                           const POP_ENTITY_NAME(vbk_block) * vbk_block,
                           POP_ENTITY_NAME(validation_state) * state) {
  VBK_ASSERT(self);
  VBK_ASSERT(vbk_block);
  VBK_ASSERT(state);

  auto res = self->ref->getMemPool().submit<altintegration::VbkBlock>(vbk_block->ref, state->ref);
  return handleSubmitResponse(res);
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           int,
                           submit_vtb,
                           const POP_ENTITY_NAME(vtb) * vtb,
                           POP_ENTITY_NAME(validation_state) * state) {
  VBK_ASSERT(self);
  VBK_ASSERT(vtb);
  VBK_ASSERT(state);

  auto res = self->ref->getMemPool().submit<altintegration::VTB>(vtb->ref, state->ref);
  return handleSubmitResponse(res);
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           int,
                           submit_atv,
                           const POP_ENTITY_NAME(atv) * atv,
                           POP_ENTITY_NAME(validation_state) * state) {
  VBK_ASSERT(self);
  VBK_ASSERT(atv);
  VBK_ASSERT(state);

  auto res = self->ref->getMemPool().submit<altintegration::ATV>(atv->ref, state->ref);
  return handleSubmitResponse(res);
}


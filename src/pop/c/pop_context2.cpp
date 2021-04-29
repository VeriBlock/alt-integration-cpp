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

  return true;
}

// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <string.h>

#include <memory>

#include "validation_state.hpp"
#include "veriblock/pop/assert.hpp"

POP_ENTITY_FREE_SIGNATURE(validation_state) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_NEW_FUNCTION(validation_state) {
  return new POP_ENTITY_NAME(validation_state);
}

POP_ENTITY_GETTER_FUNCTION(validation_state,
                           POP_ARRAY_NAME(string),
                           error_message) {
  VBK_ASSERT(self);

  std::string message = self->ref.GetDebugMessage();

  POP_ARRAY_NAME(string) res;
  res.size = message.size();
  res.data = new char[res.size];
  strncpy(res.data, message.c_str(), res.size);

  return res;
}

POP_ENTITY_GETTER_FUNCTION(validation_state, bool, is_valid) {
  VBK_ASSERT(self);

  return self->ref.IsValid();
}

POP_ENTITY_GETTER_FUNCTION(validation_state, bool, is_invalid) {
  VBK_ASSERT(self);

  return self->ref.IsInvalid();
}
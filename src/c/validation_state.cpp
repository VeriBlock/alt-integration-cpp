// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "validation_state.hpp"
#include "veriblock/assert.hpp"
#include "veriblock/c/validation_state.h"

const char* VBK_ValidationState_getErrorMessage(VBK_ValidationState* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->validation_state);

  return self->validation_state->toString().c_str();
}
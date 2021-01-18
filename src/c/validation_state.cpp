// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "validation_state.hpp"
#include "veriblock/assert.hpp"
#include "veriblock/c/validation_state.h"

VbkValidationState* VBK_NewValidationState() {
  return new VbkValidationState();
}

void VBK_FreeValidationState(VbkValidationState* self) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

const char* VBK_ValidationState_getErrorMessage(VbkValidationState* self) {
  VBK_ASSERT(self);

  return self->GetErrorMessage();
}

bool VBK_ValidationState_isValid(VbkValidationState* self) {
  VBK_ASSERT(self);

  return self->IsValid();
}

bool VBK_ValidationState_isInvalid(VbkValidationState* self) {
  VBK_ASSERT(self);

  return self->IsInvalid();
}

void VBK_ValidationState_Reset(VbkValidationState* self) {
  VBK_ASSERT(self);

  self->Reset();
}
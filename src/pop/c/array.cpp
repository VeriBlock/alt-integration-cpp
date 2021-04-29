// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/pop/c/array.h"

POP_ARRAY_FREE_SIGNATURE(u8) {
  if (self != nullptr) {
    delete[] self->data;
    self = nullptr;
  }
}

POP_ARRAY_FREE_SIGNATURE(const_u8) {
  if (self != nullptr) {
    delete[] self->data;
    self = nullptr;
  }
}

POP_ARRAY_FREE_SIGNATURE(u32) {
  if (self != nullptr) {
    delete[] self->data;
    self = nullptr;
  }
}

POP_ARRAY_FREE_SIGNATURE(const_u32) {
  if (self != nullptr) {
    delete[] self->data;
    self = nullptr;
  }
}

POP_ARRAY_FREE_SIGNATURE(string) {
  if (self != nullptr) {
    delete[] self->data;
    self = nullptr;
  }
}

POP_ARRAY_FREE_SIGNATURE(const_string) {
  if (self != nullptr) {
    delete[] self->data;
    self = nullptr;
  }
}
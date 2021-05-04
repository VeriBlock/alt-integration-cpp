// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/pop/c/array.h"

POP_ARRAY_NEW_SIGNATURE(u8) {
  POP_ARRAY_NAME(u8) res;
  res.size = size;
  res.data = new uint8_t[size];
  return res;
}

POP_ARRAY_FREE_SIGNATURE(u8) {
  if (self != nullptr) {
    delete[] self->data;
    self = nullptr;
  }
}

POP_ARRAY_NEW_SIGNATURE(u32) {
  POP_ARRAY_NAME(u32) res;
  res.size = size;
  res.data = new uint32_t[size];
  return res;
}

POP_ARRAY_FREE_SIGNATURE(u32) {
  if (self != nullptr) {
    delete[] self->data;
    self = nullptr;
  }
}

POP_ARRAY_NEW_SIGNATURE(string) {
  POP_ARRAY_NAME(string) res;
  res.size = size;
  res.data = new char[size];
  return res;
}

POP_ARRAY_FREE_SIGNATURE(string) {
  if (self != nullptr) {
    delete[] self->data;
    self = nullptr;
  }
}

POP_ARRAY_FREE_SIGNATURE(array_u8) {
  if (self != nullptr) {
    for (size_t i = 0; i < self->size; ++i) {
      delete[] self->data[i].data;
    }
    delete[] self->data;
    self = nullptr;
  }
}
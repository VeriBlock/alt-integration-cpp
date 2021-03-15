// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_SERDE_HPP__
#define VERIBLOCK_POP_CPP_SERDE_HPP__

#include <veriblock/pop/c/v2/details/serde.h>

namespace c {

template <typename T>
pop_array_u8_t* SerializeToRaw(const T& t) {
  altintegration::WriteStream w;
  t.toRaw(w);

  auto& data = w.data();
  auto* out = new pop_array_u8_t;

  out->data = new uint8_t[data.size()];
  std::copy(data.begin(), data.end(), out->data);

  out->size = data.size();
}

template <typename T>
pop_array_u8_t* SerializeToVbkEncoding(const T& t) {
  altintegration::WriteStream w;
  t.toVbkEncoding(w);

  auto& data = w.data();
  auto* out = new pop_array_u8_t;

  out->data = new uint8_t[data.size()];
  std::copy(data.begin(), data.end(), out->data);

  out->size = data.size();
}


}  // namespace c

#endif  // VERIBLOCK_POP_CPP_SERDE_HPP__

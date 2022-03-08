// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <type_traits>

#include "veriblock/pop/entities/address.hpp"
#include "veriblock/pop/serde.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
  using namespace altintegration;

  ReadStream stream(Data, Size);
  ValidationState state;

  Address addr;
  if (DeserializeFromVbkEncoding(stream, addr, state)) {
    VBK_ASSERT(state.IsValid());

    // address is supposedly valid
    auto s = addr.toString();
    VBK_ASSERT(s[0] == 'V');

    Address a;
    bool ok = a.fromString(s, state);
    VBK_ASSERT(ok);
    VBK_ASSERT(state.IsValid());
    VBK_ASSERT(addr == a);

    Address b = Address::assertFromString(s);
    VBK_ASSERT(addr == b);
  }

  return 0;
}

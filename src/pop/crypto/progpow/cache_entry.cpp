// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/crypto/progpow/cache_entry.hpp>

namespace altintegration {

void CacheEntry::toVbkEncoding(WriteStream& stream) const {}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                CacheEntry& out,
                                ValidationState& state) {
  return true;
}

}  // namespace altintegration
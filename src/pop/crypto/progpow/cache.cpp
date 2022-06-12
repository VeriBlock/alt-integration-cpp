// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/crypto/progpow/cache.hpp>

namespace altintegration {

void CacheEntry::toVbkEncoding(WriteStream& stream) const {
  // TODO implement
  assert(false && "unimplemented");
  writeContainer(
      stream, this->dag, [](WriteStream& stream, const uint32_t& value) {
        stream.writeBE(value);
      });
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                CacheEntry& out,
                                ValidationState& state) {
  // TODO implement
  assert(false && "unimplemented");
  size_t i = 0;
  if (!readArrayOf<uint32_t>(
          stream,
          out.dag,
          state,
          0,
          std::numeric_limits<size_t>::max(),
          [&i](ReadStream& stream, uint32_t& out, ValidationState& state) {
            ++i;
            return stream.readBE(out, state);
          })) {
    return state.Invalid("cache-entry-dag", i);
  }
  return true;
}

}  // namespace altintegration
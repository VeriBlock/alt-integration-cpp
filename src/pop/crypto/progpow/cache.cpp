// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/crypto/progpow/cache.hpp>

namespace altintegration {

void CacheEntry::toVbkEncoding(WriteStream& stream) const {
  stream.writeBE<uint64_t>(this->light->cache_size);
  stream.writeBE<uint64_t>(this->light->epoch);
  stream.write(this->light->cache, this->light->cache_size);

  writeContainer(
      stream, this->dag, [](WriteStream& stream, const uint32_t& value) {
        stream.writeBE(value);
      });
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                CacheEntry& out,
                                ValidationState& state) {
  if (!stream.readBE<uint64_t>(out.light->cache_size, state)) {
    return state.Invalid("invalid-cache_size");
  }
  if (!stream.readBE<uint64_t>(out.light->epoch, state)) {
    return state.Invalid("invalid-epoch");
  }
  if (!stream.read(out.light->cache_size, (uint8_t*)out.light->cache, state)) {
    return state.Invalid("invalid-cache");
  }

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
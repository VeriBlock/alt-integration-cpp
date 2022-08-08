// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_CACHE_ENTRY
#define VERIBLOCK_POP_CPP_CACHE_ENTRY

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "veriblock/pop/crypto/progpow/ethash.hpp"
#include "veriblock/pop/serde.hpp"
#include "veriblock/pop/uint.hpp"
#include "veriblock/pop/validation_state.hpp"

namespace altintegration {
class ValidationState;
class WriteStream;
namespace progpow {
struct ethash_cache;
}  // namespace progpow
struct ReadStream;

struct CacheEntry {
  std::shared_ptr<progpow::ethash_cache> light = nullptr;
  std::vector<uint32_t> dag;

  void toVbkEncoding(WriteStream& stream) const;
};

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                CacheEntry& out,
                                ValidationState& state);

struct EthashCacheI {
  virtual ~EthashCacheI() = default;

  virtual std::shared_ptr<CacheEntry> getOrDefault(
      uint64_t epoch, std::function<std::shared_ptr<CacheEntry>()> factory) = 0;

  virtual void clear() = 0;
};

struct ProgpowHeaderCacheI {
  virtual ~ProgpowHeaderCacheI() = default;

  virtual void insert(const uint256& key, uint192 value) = 0;

  virtual bool tryGet(const uint256& key, uint192& value) = 0;

  virtual void clear() = 0;
};

}  // namespace altintegration

#endif

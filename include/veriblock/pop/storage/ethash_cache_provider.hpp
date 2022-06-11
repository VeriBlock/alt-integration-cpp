// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ETHASH_CACHE_PROVIDER_HPP
#define VERIBLOCK_POP_CPP_ETHASH_CACHE_PROVIDER_HPP

#include <veriblock/pop/crypto/progpow/cache_entry.hpp>

namespace altintegration {

struct EthashCache {
  virtual ~EthashCache() = default;

  //! @pure
  virtual bool get(uint64_t epoch, std::shared_ptr<CacheEntry> out) const = 0;

  //! @pure
  virtual void insert(uint64_t epoch,
                      std::shared_ptr<CacheEntry> value) = 0;

  std::shared_ptr<CacheEntry> getOrDefault(
      uint64_t epoch, std::function<std::shared_ptr<CacheEntry>()> factory) {
    std::shared_ptr<CacheEntry> value;
    if (!get(epoch, value)) {
      value = factory();
      insert(epoch, value);
    }

    return value;
  }
};

}  // namespace altintegration

#endif
// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_ADAPTORS_ETHASH_CACHE_PROVIDER_IMPL_HPP
#define VERIBLOCK_POP_CPP_STORAGE_ADAPTORS_ETHASH_CACHE_PROVIDER_IMPL_HPP

#include <veriblock/pop/storage/ethash_cache_provider.hpp>

#include "storage_interface.hpp"

namespace altintegration {

namespace adaptors {

struct EthashCacheImpl : public EthashCache {
  ~EthashCacheImpl() override = default;

  EthashCacheImpl(Storage& storage) : storage_(storage) {}

  bool get(uint64_t epoch, std::shared_ptr<CacheEntry> out) const override {
    std::vector<uint8_t> bytes_out;

    return true;
  }

  void insert(uint64_t epoch, std::shared_ptr<CacheEntry> value) override {}

 private:
  Storage& storage_;
};

}  // namespace adaptors

}  // namespace altintegration

#endif
// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_ADAPTORS_ETHASH_CACHE_PROVIDER_IMPL_HPP
#define VERIBLOCK_POP_CPP_STORAGE_ADAPTORS_ETHASH_CACHE_PROVIDER_IMPL_HPP

#include <veriblock/pop/storage/ethash_cache_provider.hpp>
#include <veriblock/pop/validation_state.hpp>

#include "storage_interface.hpp"

namespace altintegration {

namespace adaptors {

const char DB_ETHASH_PREFIX = '&';

inline std::vector<uint8_t> epoch_bytes(uint64_t epoch) {
  WriteStream write;
  write.writeBE(epoch);
  auto res = write.data();
  res.insert(res.begin(), DB_ETHASH_PREFIX);
  return res;
}

struct EthashCacheImpl : public EthashCache {
  ~EthashCacheImpl() override = default;

  EthashCacheImpl(Storage& storage) : storage_(storage) {}

  bool get(uint64_t epoch, std::shared_ptr<CacheEntry> out) const override {
    std::vector<uint8_t> bytes_out;
    if (!storage_.read(epoch_bytes(epoch), bytes_out)) {
      return false;
    }

    ReadStream read(bytes_out);
    ValidationState state;
    if (!DeserializeFromVbkEncoding(read, *out, state)) {
      return false;
    }

    return true;
  }

  void insert(uint64_t epoch, std::shared_ptr<CacheEntry> value) override {
    WriteStream write;
    value->toVbkEncoding(write);

    storage_.write(epoch_bytes(epoch), write.data());
  }

 private:
  Storage& storage_;
};

}  // namespace adaptors

}  // namespace altintegration

#endif

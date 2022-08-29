// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_ADAPTORS_PROGPOW_HEADER_CACHE_PROVIDER_IMPL_HPP
#define VERIBLOCK_POP_CPP_STORAGE_ADAPTORS_PROGPOW_HEADER_CACHE_PROVIDER_IMPL_HPP

#include <veriblock/pop/storage/progpow_header_cache_provider.hpp>
#include <veriblock/pop/validation_state.hpp>

#include "storage_interface.hpp"

namespace altintegration {

namespace adaptors {

const char DB_PROGPOW_HEADER_PREFIX = '!';

inline std::vector<uint8_t> key_bytes(const uint256& key) {
  auto res = key.asVector();
  res.insert(res.begin(), DB_PROGPOW_HEADER_PREFIX);
  return res;
}

struct ProgpowHeaderCacheImpl : public ProgpowHeaderCache {
  ~ProgpowHeaderCacheImpl() override = default;

  ProgpowHeaderCacheImpl(Storage& storage) : storage_(storage) {}

  bool get(const uint256& key, uint192& value) const override {
    std::vector<uint8_t> bytes_out;
    if (!storage_.read(key_bytes(key), bytes_out)) {
      return false;
    }
    value = bytes_out;

    return true;
  }

  void insert(const uint256& key, uint192 value) override {
    storage_.write(key_bytes(key), value.asVector());
  }

 private:
  Storage& storage_;
};
}  // namespace adaptors

}  // namespace altintegration

#endif

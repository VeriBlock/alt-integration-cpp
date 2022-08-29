// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_PROGPOW_HEADER_CACHE_PROVIDER_HPP
#define VERIBLOCK_POP_CPP_PROGPOW_HEADER_CACHE_PROVIDER_HPP

#include <veriblock/pop/crypto/progpow/cache.hpp>

namespace altintegration {

struct ProgpowHeaderCache : public ProgpowHeaderCacheI {
  virtual ~ProgpowHeaderCache() = default;

  //! @pure
  virtual bool get(const uint256& key, uint192& value) const = 0;

  bool tryGet(const uint256& key, uint192& value) override {
    return this->get(key, value);
  }

  void clear() override {}
};

}  // namespace altintegration

#endif

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
  virtual bool get(uint64_t epoch, CacheEntry& out) const = 0;
};

}

#endif
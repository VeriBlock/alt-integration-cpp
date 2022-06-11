// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_CACHE_ENTRY
#define VERIBLOCK_POP_CPP_CACHE_ENTRY

#include <memory>
#include <veriblock/pop/crypto/progpow/ethash.hpp>

namespace altintegration {

struct CacheEntry {
  std::shared_ptr<progpow::ethash_cache> light = nullptr;
  std::vector<uint32_t> dag;
};

}

#endif
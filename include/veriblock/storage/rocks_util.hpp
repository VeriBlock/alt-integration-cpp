// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ROCKS_UTIL_HPP_

#include <rocksdb/slice.h>

#include <vector>
#include <veriblock/slice.hpp>

namespace altintegration {

inline rocksdb::Slice makeRocksSlice(const std::vector<uint8_t>& vec) {
  rocksdb::Slice res(reinterpret_cast<const char*>(vec.data()), vec.size());
  return res;
}

inline rocksdb::Slice makeRocksSlice(const Slice<const uint8_t>& slice) {
  rocksdb::Slice res(reinterpret_cast<const char*>(slice.data()), slice.size());
  return res;
}

}  // namespace altintegration

#endif

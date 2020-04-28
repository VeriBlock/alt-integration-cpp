// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_BASE59_HPP
#define ALT_INTEGRATION_VERIBLOCK_BASE59_HPP

#include <string>
#include <vector>

namespace altintegration {

std::string EncodeBase59(const unsigned char *pbegin,
                         const unsigned char *pend);

std::string EncodeBase59(const uint8_t *buf, size_t size);

template <typename T,
          typename = typename std::enable_if<sizeof(typename T::value_type) ==
                                             1>::type>
std::string EncodeBase59(const T &container) {
  const uint8_t *ptr = container.data();
  return EncodeBase59(ptr, ptr + container.size());
}

std::vector<uint8_t> DecodeBase59(const std::string &str);

}  // namespace altintegration

#endif

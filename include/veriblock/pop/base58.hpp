// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_BASE58_HPP
#define ALT_INTEGRATION_VERIBLOCK_BASE58_HPP

#include <string>
#include <vector>

namespace altintegration {

//! Encodes input bytes to Base58
std::string EncodeBase58(const unsigned char *pbegin,
                         const unsigned char *pend);

//! @overload
template <typename T,
          typename = typename std::enable_if<sizeof(typename T::value_type) ==
                                             1>::type>
std::string EncodeBase58(const T &container) {
  const uint8_t *ptr = container.data();
  return EncodeBase58(ptr, ptr + container.size());
}

//! Decodes base58 into bytes
bool DecodeBase58(const std::string &str, std::vector<unsigned char> &out);

//! @overload
//! @note Will die on assert if input string is not base58.
std::vector<uint8_t> AssertDecodeBase58(const std::string &str);

}  // namespace altintegration

#endif

// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_LITERAPS_HPP
#define ALT_INTEGRATION_LITERAPS_HPP

#include <vector>
#include <veriblock/pop/strutil.hpp>

namespace altintegration {

//! @private
inline std::vector<uint8_t> operator""_unhex(const char *str, size_t size) {
  std::string hex{str, str + size};
  return altintegration::ParseHex(hex);
}

//! @private
inline std::vector<uint8_t> operator""_v(const char *str, size_t size) {
  return std::vector<uint8_t>{str, str + size};
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_LITERAPS_HPP

// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_HASHERS_HPP
#define VERIBLOCK_POP_CPP_HASHERS_HPP

#include <cstdint>
#include <string>
#include <vector>

template <>
struct std::hash<std::vector<uint8_t>> {
  size_t operator()(const std::vector<uint8_t>& x) const {
    return std::hash<std::string>{}(std::string{x.begin(), x.end()});
  }
};

#endif  // VERIBLOCK_POP_CPP_HASHERS_HPP

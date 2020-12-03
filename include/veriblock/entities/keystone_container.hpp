// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_KEYSTONE_CONTAINER_HPP
#define VERIBLOCK_POP_CPP_KEYSTONE_CONTAINER_HPP

#include <vector>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/altblock.hpp>

namespace altintegration {

struct KeystoneContainer {
  std::vector<uint8_t> firstPreviousKeystone;
  std::vector<uint8_t> secondPreviousKeystone;

  static KeystoneContainer fromPrevious(const BlockIndex<AltBlock>* prev,
                                        const uint32_t keystoneInterval);

  void write(WriteStream& stream) const;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_KEYSTONE_CONTAINER_HPP

// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_UINT_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_UINT_HPP_

#include "veriblock/blob.hpp"
#include "veriblock/consts.hpp"

namespace altintegration {

using uint128 = Blob<VBK_MERKLE_ROOT_HASH_SIZE>;
using uint256 = Blob<SHA256_HASH_SIZE>;
using uint72 = Blob<VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE>;
using uint96 = Blob<VBLAKE_PREVIOUS_BLOCK_HASH_SIZE>;
using uint192 = Blob<VBLAKE_BLOCK_HASH_SIZE>;
}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_UINT_HPP_

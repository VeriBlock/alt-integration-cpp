// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_UINT_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_UINT_HPP_

#include "blob.hpp"
#include "consts.hpp"

namespace altintegration {

extern template struct Blob<VBK_MERKLE_ROOT_HASH_SIZE>;
extern template struct Blob<SHA256_HASH_SIZE>;
extern template struct Blob<VBK_PREVIOUS_KEYSTONE_HASH_SIZE>;
extern template struct Blob<VBK_PREVIOUS_BLOCK_HASH_SIZE>;
extern template struct Blob<VBK_BLOCK_HASH_SIZE>;

//! Fixed-size array.
using uint128 = Blob<VBK_MERKLE_ROOT_HASH_SIZE>;
//! @overload
using uint256 = Blob<SHA256_HASH_SIZE>;
//! @overload
using uint72 = Blob<VBK_PREVIOUS_KEYSTONE_HASH_SIZE>;
//! @overload
using uint96 = Blob<VBK_PREVIOUS_BLOCK_HASH_SIZE>;
//! @overload
using uint192 = Blob<VBK_BLOCK_HASH_SIZE>;
}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_UINT_HPP_

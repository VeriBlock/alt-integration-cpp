#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_HASHES_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_HASHES_HPP_

#include "veriblock/blob.hpp"
#include "veriblock/consts.hpp"

namespace VeriBlock {

using uint128 = Blob<VBK_MERKLE_ROOT_HASH_SIZE>;
using uint256 = Blob<SHA256_HASH_SIZE>;
using uint72 = Blob<VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE>;
using uint144 = Blob<VBLAKE_PREVIOUS_BLOCK_HASH_SIZE>;
using uint192 = Blob<VBLAKE_BLOCK_HASH_SIZE>;
}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_HASHES_HPP_

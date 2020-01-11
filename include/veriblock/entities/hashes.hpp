#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_HASHES_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_HASHES_HPP_

#include "veriblock/blob.hpp"
#include "veriblock/consts.hpp"

namespace VeriBlock {

using VbkMerkleRootSha256Hash = Blob<VBK_MERKLE_ROOT_SIZE>;
using Sha256Hash = Blob<SHA256_HASH_SIZE>;
using VBlakePrevKeystoneHash = Blob<VBLAKE_PREVIOUS_KEYSTONE_SIZE>;
using VBlakeBlockHash = Blob<VBLAKE_PREVIOUS_BLOCK_SIZE>;

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_HASHES_HPP_

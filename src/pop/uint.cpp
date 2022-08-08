#include <veriblock/pop/uint.hpp>

#include "veriblock/pop/blob.hpp"
#include "veriblock/pop/consts.hpp"

namespace altintegration {

template struct Blob<VBK_MERKLE_ROOT_HASH_SIZE>;
template struct Blob<SHA256_HASH_SIZE>;
template struct Blob<VBK_PREVIOUS_KEYSTONE_HASH_SIZE>;
template struct Blob<VBK_PREVIOUS_BLOCK_HASH_SIZE>;
template struct Blob<VBK_BLOCK_HASH_SIZE>;

}  // namespace altintegration
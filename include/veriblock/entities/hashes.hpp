#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_HASHES_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_HASHES_HPP_

#include "veriblock/blob.hpp"

namespace VeriBlock {

using Sha256Hash = Blob<32>;
using VBlakePrevKeystoneHash = Blob<9>;
using VBlakeBlockHash = Blob<12>;

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_HASHES_HPP_

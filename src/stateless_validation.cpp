

#include <veriblock/entities/hashes.hpp>
#include <veriblock/stateless_validation.hpp>

namespace VeriBlock {

bool checkProofOfWork(const BtcBlock& block) {
  Sha256Hash blockHash = block.getBlockHash();
  Sha256Hash target = decodeBits(block.bits);

  return target > blockHash;
}

bool checkBtcBlock(const BtcBlock& block, ValidationState& state) {
  if (!checkProofOfWork(block)) {
    return state.Invalid("checkBtcBlock is failed",
                         "Block hash is higher than target");
  }

  return true;
}
}  // namespace VeriBlock

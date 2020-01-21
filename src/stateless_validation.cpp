
#include <veriblock/stateless_validation.hpp>

namespace VeriBlock {

bool checkProofOfWork(const BtcBlock& block) { return block.version; }

bool checkBtcBlock(const BtcBlock& block, ValidationState& state) {
  if (!checkProofOfWork(block)) {
    return state.Invalid("checkBtcBlock is failed",
                         "Bitcoin block bad checkProofOfWork");
  }

  return true;
}
}  // namespace VeriBlock

#include "veriblock/arith_uint256.hpp"
#include "veriblock/consts.hpp"
#include "veriblock/stateless_validation.hpp"

namespace VeriBlock {

static const ArithUint256 MAXIMUM_DIFFICULTY(
    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");

bool checkProofOfWork(const BtcBlock& block) {
  ArithUint256 blockHash(block.getHash());
  ArithUint256 target;
  target.decodeBits(block.bits);
  return target > blockHash;
}

bool checkProofOfWork(const VbkBlock& block) {
  ArithUint256 blockHash(block.getHash());
  ArithUint256 target;
  target.decodeBits(block.difficulty);
  target = MAXIMUM_DIFFICULTY / target;
  return target > blockHash;
}

bool checkBtcBlock(const BtcBlock& block, ValidationState& state) {
  if (!checkProofOfWork(block)) {
    return state.Invalid("checkBtcBlock", "Block hash is higher than target");
  }

  if (!checkMaximumDrift(block)) {
    return state.Invalid("checkBtcBlock", "Block is too far in the future");
  }

  return true;
}

bool checkVbkBlock(const VbkBlock& block, ValidationState& state) {
  if (!checkProofOfWork(block)) {
    return state.Invalid("checkVbkBlock", "Block hash is higher than target");
  }

  if (!checkMaximumDrift(block)) {
    return state.Invalid("checkVbkBlock", "Block is too far in the future");
  }

  return true;
}
}  // namespace VeriBlock

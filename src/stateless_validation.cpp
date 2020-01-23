#include <veriblock/blockutils.hpp>
#include <veriblock/consts.hpp>
#include <veriblock/entities/hashes.hpp>
#include <veriblock/stateless_validation.hpp>
#include <veriblock/time.hpp>

namespace VeriBlock {

bool checkProofOfWork(const BtcBlock& block) {
  uint256 blockHash = block.getHash();
  uint256 target = decodeBits(block.bits);

  return target > blockHash;
}

bool checkProofOfWork(const VbkBlock& block) {
  // TODO : write the target calculation
  uint192 blockHash = block.getHash();
  uint256 target = decodeBits(block.difficulty);

  return true;
}

template <typename BlockType>
bool checkMaximumDrift(const BlockType& block) {
  uint32_t currentTime = currentTimestamp4();
  return block.timestamp < currentTime + ALLOWED_TIME_DRIFT;
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

#include <time.h>

#include <veriblock/consts.hpp>
#include <veriblock/entities/hashes.hpp>
#include <veriblock/stateless_validation.hpp>

namespace {

uint32_t getCurrentTime() { return (uint32_t)time(0); }

}  // namespace

namespace VeriBlock {

bool checkProofOfWork(const BtcBlock& block) {
  uint256 blockHash = block.getBlockHash();
  uint256 target = decodeBits(block.bits);

  return target > blockHash;
}

bool checkProofOfWork(const VbkBlock& block) {
  // TODO : write the target calculation
  uint192 blockHash = block.getBlockHash();
  uint256 target = decodeBits(block.difficulty);

  return true;
}

template <typename BlockType>
bool checkMaximumDrift(const BlockType& block) {
  uint32_t currentTime = ::getCurrentTime();
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

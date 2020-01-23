#include <time.h>

#include <veriblock/consts.hpp>
#include <veriblock/entities/hashes.hpp>
#include <veriblock/stateless_validation.hpp>

namespace VeriBlock {

bool checkProofOfWork(const BtcBlock& block) {
  Sha256Hash blockHash = block.getBlockHash();
  Blob<SHA256_HASH_SIZE> target = decodeBits<SHA256_HASH_SIZE>(block.bits);

  return target > blockHash;
}

bool checkProofOfWork(const VbkBlock& block) {
  VBlakeBlockHash blockHash = block.getBlockHash();
  Blob<VBLAKE_BLOCK_SIZE> target =
      decodeBits<VBLAKE_BLOCK_SIZE>(block.difficulty);

  return target > blockHash;
}

template <typename BlockType>
bool checkMaximumDrift(const BlockType& block) {
  uint32_t currentTime = (uint32_t)time(0);
  return block.timestamp > currentTime + ALLOWED_TIME_DRIFT;
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

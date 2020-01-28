#ifndef ALT_INTEGRATION_VERIBLOCK_STATELESS_VALIDATION_H
#define ALT_INTEGRATION_VERIBLOCK_STATELESS_VALIDATION_H

#include "entities/vbkblock.hpp"
#include "validation_state.hpp"
#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/time.hpp"

namespace VeriBlock {

bool checkProofOfWork(const BtcBlock& block);

bool checkProofOfWork(const VbkBlock& block);

template <typename BlockType>
bool checkMaximumDrift(const BlockType& block) {
  uint32_t currentTime = currentTimestamp4();
  return (uint32_t)block.timestamp < currentTime + ALLOWED_TIME_DRIFT;
}

template <typename MerklePathType, typename HashType1, typename HashType2>
bool checkMerklePath(const MerklePathType& merklePath,
                     const HashType1& transactionHash,
                     const HashType2& merkleRoot,
                     ValidationState& state) {
  if (merklePath.subject != transactionHash) {
    return state.Invalid("checkMerklePath()",
                         "Transaction hash cannot be proven by merkle path");
  }

  if (merklePath.calculateMerkleRoot() != merkleRoot) {
    return state.Invalid("checkMerklePath()",
                         "merkle path does not belong to block hash");
  }

  return true;
}

bool checkBtcBlock(const BtcBlock& block, ValidationState& state);

bool checkVbkBlock(const VbkBlock& block, ValidationState& state);

}  // namespace VeriBlock

#endif  // ! ALT_INTEGRATION_INCLUDE_VERIBLOCK_STATELESS_VALIDATION_H

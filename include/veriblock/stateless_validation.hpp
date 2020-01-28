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

template <typename PublicationType>
bool checkMerklePath(const PublicationType& publication,
                     ValidationState& state) {
  if (publication.merklePath.subject != publication.transaction.getHash()) {
    return state.Invalid(
        "checkMerklePath()",
        "VeriBlock PoP Transaction cannot be proven by merkle path");
  }

  if (publication.merklePath.calculateMerkleRoot() !=
      publication.containingBlock.merkleRoot) {
    return state.Invalid(
        "checkMerklePath()",
        "VeriBlock PoP transaction does not belong to containing block");
  }

  return true;
}

bool checkBtcBlock(const BtcBlock& block, ValidationState& state);

bool checkVbkBlock(const VbkBlock& block, ValidationState& state);

}  // namespace VeriBlock

#endif  // ! ALT_INTEGRATION_INCLUDE_VERIBLOCK_STATELESS_VALIDATION_H

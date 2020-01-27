#ifndef ALT_INTEGRATION_VERIBLOCK_STATELESS_VALIDATION_H
#define ALT_INTEGRATION_VERIBLOCK_STATELESS_VALIDATION_H

#include "entities/vbkblock.hpp"
#include "validation_state.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/time.hpp"

namespace VeriBlock {

bool checkProofOfWork(const BtcBlock& block);

bool checkProofOfWork(const VbkBlock& block);

template <typename BlockType>
bool checkMaximumDrift(const BlockType& block) {
  uint32_t currentTime = currentTimestamp4();
  return (uint32_t)block.timestamp < currentTime + ALLOWED_TIME_DRIFT;
}

bool checkBtcBlock(const BtcBlock& block, ValidationState& state);

bool checkVbkBlock(const VbkBlock& block, ValidationState& state);

}  // namespace VeriBlock

#endif  // ! ALT_INTEGRATION_INCLUDE_VERIBLOCK_STATELESS_VALIDATION_H



#include "entities/btcblock.hpp"
#include "entities/vbkblock.hpp"
#include "validation_state.hpp"

namespace VeriBlock {

bool checkBtcBlock(const BtcBlock& block, ValidationState& state);

bool checkVbkBlock(const VbkBlock& block, ValidationState& state);

}  // namespace VeriBlock

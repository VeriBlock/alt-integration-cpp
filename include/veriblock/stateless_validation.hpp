#ifndef ALT_INTEGRATION_VERIBLOCK_STATELESS_VALIDATION_H
#define ALT_INTEGRATION_VERIBLOCK_STATELESS_VALIDATION_H

#include "entities/vbkblock.hpp"
#include "validation_state.hpp"
#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/signutil.hpp"
#include "veriblock/time.hpp"

namespace VeriBlock {

template <typename BlockType>
bool checkMaximumDrift(const BlockType& block, ValidationState& state) {
  uint32_t currentTime = currentTimestamp4();
  if ((uint32_t)block.timestamp >= currentTime + ALLOWED_TIME_DRIFT) {
    return state.Invalid("checkMaximumDrift()",
                         "Invalid Block",
                         "Block is too far in the future");
  }
  return true;
}

template <typename MerklePathType, typename HashType1, typename HashType2>
bool checkMerklePath(const MerklePathType& merklePath,
                     const HashType1& transactionHash,
                     const HashType2& merkleRoot,
                     ValidationState& state) {
  if (merklePath.subject != transactionHash) {
    return state.Invalid("checkMerklePath()",
                         "Invalid MerklePath",
                         "Transaction hash cannot be proven by merkle path");
  }

  if (merklePath.calculateMerkleRoot() != merkleRoot) {
    return state.Invalid("checkMerklePath()",
                         "Invalid MerklePath",
                         "merkle path does not belong to block hash");
  }

  return true;
}

bool checkSignature(const VbkTx& tx, ValidationState& state);

bool checkSignature(const VbkPopTx& tx, ValidationState& state);

bool checkBtcBlocks(const std::vector<BtcBlock>& btcBlocks,
                    ValidationState& state);

bool checkVbkBlocks(const std::vector<VbkBlock>& vbkBlocks,
                    ValidationState& state);

bool checkBitcoinTransactionForPoPData(const VbkPopTx& tx,
                                       ValidationState& state);

bool checkProofOfWork(const BtcBlock& block, ValidationState& state);

bool checkProofOfWork(const VbkBlock& block, ValidationState& state);

bool checkVbkPopTx(const VbkPopTx& tx, ValidationState& state);

bool checkVbkTx(const VbkTx& tx, ValidationState& state);

bool checkBtcBlock(const BtcBlock& block, ValidationState& state);

bool checkVbkBlock(const VbkBlock& block, ValidationState& state);

bool checkATV(const ATV& atv, ValidationState& state);

bool checkVTB(const VTB& vtb, ValidationState& state);
}  // namespace VeriBlock

#endif  // ! ALT_INTEGRATION_INCLUDE_VERIBLOCK_STATELESS_VALIDATION_H

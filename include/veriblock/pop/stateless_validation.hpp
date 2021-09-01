// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_STATELESS_VALIDATION_H
#define ALT_INTEGRATION_VERIBLOCK_STATELESS_VALIDATION_H

#include "blockchain/alt_chain_params.hpp"
#include "blockchain/btc_chain_params.hpp"
#include "blockchain/vbk_chain_params.hpp"
#include "crypto/secp256k1.hpp"
#include "entities/atv.hpp"
#include "entities/btcblock.hpp"
#include "entities/popdata.hpp"
#include "entities/vbkblock.hpp"
#include "entities/vtb.hpp"
#include "pop_stateless_validator.hpp"
#include "time.hpp"
#include "validation_state.hpp"

namespace altintegration {

//! @private
bool containsSplit(const std::vector<uint8_t>& pop_data,
                   const std::vector<uint8_t>& btcTx_data,
                   ValidationState& state);

//! Verify merkle path.
template <typename MerklePathType, typename HashType1, typename HashType2>
bool checkMerklePath(const MerklePathType& merklePath,
                     const HashType1& transactionHash,
                     const HashType2& merkleRoot,
                     ValidationState& state) {
  if (merklePath.subject != transactionHash) {
    return state.Invalid("invalid-merklepath",
                         "Transaction hash cannot be proven by merkle path");
  }

  auto root = merklePath.calculateMerkleRoot();
  if (root != merkleRoot) {
    return state.Invalid("invalid-merklepath",
                         "Wrong merkle root. Expected: " + merkleRoot.toHex() +
                             ", got: " + root.toHex());
  }

  return true;
}

//! Verify signature of a transaction.
bool checkSignature(const VbkTx& tx, ValidationState& state);

//! @overload
bool checkSignature(const VbkPopTx& tx, ValidationState& state);

//! Stateless validation for a chain of Blocks.
bool checkBtcBlocks(const std::vector<BtcBlock>& btcBlocks,
                    ValidationState& state,
                    const BtcChainParams& param);

//! @overload
bool checkVbkBlocks(const std::vector<VbkBlock>& vbkBlocks,
                    ValidationState& state,
                    const VbkChainParams& param);

//! Search BtcTx for POP data.
bool checkBitcoinTransactionForPoPData(const VbkPopTx& tx,
                                       ValidationState& state);

//! Stateless validation for a Block.
bool checkProofOfWork(const BtcBlock& block, const BtcChainParams& param);
//! @overload
bool checkProofOfWork(const VbkBlock& block, const VbkChainParams& param);

//! Stateless validation for VbkPopTx.
bool checkVbkPopTx(const VbkPopTx& tx,
                   ValidationState& state,
                   const BtcChainParams& param,
                   const VbkChainParams& vbk);

//! Stateless validation for VbkTx.
bool checkVbkTx(const VbkTx& tx,
                const AltChainParams& params,
                const VbkChainParams& vbkparams,
                ValidationState& state);

//! Stateless validation for PublicationData.
bool checkPublicationData(const PublicationData& pub,
                          const AltChainParams& params,
                          ValidationState& state);

//! Stateless validation for a single block.
bool checkBlock(const BtcBlock& block,
                ValidationState& state,
                const BtcChainParams& params);

//! Optimization: VBK block is not plausible if its combination of height+time
//! do not correspond to potential VBK network.
bool checkVbkBlockPlausibility(const VbkBlock& block,
                               ValidationState& state,
                               const VbkChainParams& params);

//! Stateless validation for VbkBlock.
bool checkBlock(const VbkBlock& block,
                ValidationState& state,
                const VbkChainParams& params);

//! Stateless validation for ATV.
bool checkATV(const ATV& atv,
              ValidationState& state,
              const AltChainParams& alt,
              const VbkChainParams& vbkp);

//! Stateless validation for VTB.
bool checkVTB(const VTB& vtb,
              ValidationState& state,
              const BtcChainParams& btc,
              const VbkChainParams& vbk);

//! @private
template <typename P>
bool hasDuplicateIds(const std::vector<typename P::id_t>& payloadIds) {
  std::unordered_set<typename P::id_t> ids;
  for (const auto& id : payloadIds) {
    bool inserted = ids.insert(id).second;
    if (!inserted) {
      return true;
    }
  }
  return false;
}

//! @private
template <typename P>
bool checkIdsForDuplicates(const std::vector<typename P::id_t>& payloadIds,
                           ValidationState& state) {
  if (hasDuplicateIds<P>(payloadIds)) {
    return state.Invalid(P::name() + "-duplicate",
                         format("duplicate {} ids", P::name()));
  }
  return true;
}

//! @private
bool checkPopDataForDuplicates(const PopData& popData, ValidationState& state);

//! Stateless validation for PopData and all internal PoP payloads.
bool checkPopData(PopValidator& validator,
                  const PopData& popData,
                  ValidationState& state);
}  // namespace altintegration

#endif  // ! ALT_INTEGRATION_INCLUDE_VERIBLOCK_STATELESS_VALIDATION_H

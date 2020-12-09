// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_STATELESS_VALIDATION_H
#define ALT_INTEGRATION_VERIBLOCK_STATELESS_VALIDATION_H

#include "entities/vbkblock.hpp"
#include "validation_state.hpp"
#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/btc_chain_params.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/crypto/secp256k1.hpp"
#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/popdata.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/pop_stateless_validator.hpp"
#include "veriblock/time.hpp"

namespace altintegration {

bool containsSplit(const std::vector<uint8_t>& pop_data,
                   const std::vector<uint8_t>& btcTx_data, ValidationState& state);

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

bool checkSignature(const VbkTx& tx, ValidationState& state);

bool checkSignature(const VbkPopTx& tx, ValidationState& state);

bool checkBtcBlocks(const std::vector<BtcBlock>& btcBlocks,
                    ValidationState& state,
                    const BtcChainParams& param);

bool checkVbkBlocks(const std::vector<VbkBlock>& vbkBlocks,
                    ValidationState& state,
                    const VbkChainParams& param);

bool checkBitcoinTransactionForPoPData(const VbkPopTx& tx,
                                       ValidationState& state);

bool checkProofOfWork(const BtcBlock& block, const BtcChainParams& param);

bool checkProofOfWork(const VbkBlock& block, const VbkChainParams& param);

bool checkVbkPopTx(const VbkPopTx& tx,
                   ValidationState& state,
                   const BtcChainParams& param);

bool checkVbkTx(const VbkTx& tx, ValidationState& state);

bool checkBlock(const BtcBlock& block,
                ValidationState& state,
                const BtcChainParams& params);

bool checkVbkBlockPlausibility(const VbkBlock& block, ValidationState& state, const VbkChainParams& params);

bool checkBlock(const VbkBlock& block,
                ValidationState& state,
                const VbkChainParams& params);

bool checkATV(const ATV& atv,
              ValidationState& state,
              const AltChainParams& alt);

bool checkVTB(const VTB& vtb,
              ValidationState& state,
              const BtcChainParams& btc);

bool checkPopDataForDuplicates(const PopData& popData, ValidationState& state);

bool checkPopData(PopValidator& validator,
                  const PopData& popData,
                  ValidationState& state);
}  // namespace altintegration

#endif  // ! ALT_INTEGRATION_INCLUDE_VERIBLOCK_STATELESS_VALIDATION_H

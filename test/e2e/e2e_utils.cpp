// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "e2e_utils.hpp"
#include "pop/util/test_utils.hpp"
#include "veriblock/pop/alt-util.hpp"
#include "veriblock/pop/blockchain/miner.hpp"

namespace altintegration {
namespace testing_utils {

ActionOption GetRandomAction() {
  return (ActionOption)(rand() % (uint8_t)ActionOption::kMaxValue);
}

ForkOption GetRandomFork() {
  return (ForkOption)(rand() % (uint8_t)ForkOption::kMaxValue);
}

template <>
AltBlock generateRandomNextBlock(const BlockIndex<AltBlock>& previous,
                                 const AltChainParams&) {
  AltBlock block;
  block.hash = generateRandomBytesVector(32);
  block.height = previous.getHeight() + 1;
  block.previousBlock = previous.getHash();
  block.timestamp = previous.getTimestamp() + 1;
  return block;
}

template <>
VbkBlock generateRandomNextBlock(const BlockIndex<VbkBlock>& previous,
                                 const VbkChainParams& params) {
  Miner<VbkBlock, VbkChainParams> miner =
      Miner<VbkBlock, VbkChainParams>(params);

  return miner.createNextBlock(previous);
}

template <>
BtcBlock generateRandomNextBlock(const BlockIndex<BtcBlock>& previous,
                                 const BtcChainParams& params) {
  Miner<BtcBlock, BtcChainParams> miner =
      Miner<BtcBlock, BtcChainParams>(params);

  return miner.createNextBlock(previous);
}

void E2EState::applyAction(ActionOption action,
                           ForkOption fork,
                           const AltBlockTree& state) {
  switch (action) {
    case ActionOption::MINE_ALT: {
      auto& block = getBlock(fork, state);
      auto new_block = generateRandomNextBlock(block, state.getParams());
      break;
    }
    case ActionOption::MINE_VBK: {
      auto& block = getBlock(fork, state.vbk());
      auto new_block = generateRandomNextBlock(block, state.vbk().getParams());
      break;
    }
    case ActionOption::MINE_BTC: {
      auto& block = getBlock(fork, state.btc());
      auto new_block = generateRandomNextBlock(block, state.btc().getParams());
      break;
    }
    case ActionOption::CREATE_BTC_TX: {
      auto& block = getBlock(fork, state.vbk());
      auto tx = mock_miner.createBtcTxEndorsingVbkBlock(block.getHeader());
      break;
    }
    case ActionOption::CREATE_VBK_TX: {
      auto& block = getBlock(fork, state);

      auto& atvs = block.getPayloadIds<ATV>();
      auto& vtbs = block.getPayloadIds<VTB>();
      auto& vbks = block.getPayloadIds<VbkBlock>();

      PublicationData pd = GeneratePublicationData(block.getHash(),
                                                   block,
                                                   {1, 2, 3, 4, 5},
                                                   1,
                                                   atvs,
                                                   vtbs,
                                                   vbks,
                                                   {1, 2, 3, 4, 5},
                                                   state.getParams());

      auto tx = mock_miner.createVbkTxEndorsingAltBlock(pd);
      break;
    }
    case ActionOption::CREATE_VBK_POP_TX: {
      auto& block = getBlock(fork, state.vbk());
      auto tx = mock_miner.createVbkPopTxEndorsingVbkBlock(
          block.getHeader(), state.btc().getBestChain().tip()->getHash());
      break;
    }

    default:
      break;
  }
}

}  // namespace testing_utils
}  // namespace altintegration
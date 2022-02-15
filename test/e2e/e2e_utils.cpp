// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <random>

#include "e2e_utils.hpp"
#include "pop/util/test_utils.hpp"
#include "veriblock/pop/alt-util.hpp"
#include "veriblock/pop/blockchain/miner.hpp"

namespace altintegration {
namespace testing_utils {

Action GetRandomAction() {
  return (Action)(rand() % (uint8_t)Action::kMaxValue);
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

void E2EState::applyAction(Action action, const AltBlockTree& state) {
  switch (action) {
    case Action::MINE_ALT: {
      auto* tip = state.getBestChain().tip();
      auto block = generateRandomNextBlock(*tip, state.getParams());
      break;
    }
    case Action::MINE_VBK: {
      auto* tip = state.vbk().getBestChain().tip();
      auto block = generateRandomNextBlock(*tip, state.vbk().getParams());
      break;
    }
    case Action::MINE_BTC: {
      auto* tip = state.btc().getBestChain().tip();
      auto block = generateRandomNextBlock(*tip, state.btc().getParams());
      break;
    }
    case Action::CREATE_BTC_TX: {
      auto* tip = state.vbk().getBestChain().tip();
      auto tx = mock_miner.createBtcTxEndorsingVbkBlock(tip->getHeader());
      break;
    }
    case Action::CREATE_VBK_TX: {
      auto* tip = state.getBestChain().tip();

      auto& atvs = tip->getPayloadIds<ATV>();
      auto& vtbs = tip->getPayloadIds<VTB>();
      auto& vbks = tip->getPayloadIds<VbkBlock>();

      PublicationData pd = GeneratePublicationData(tip->getHash(),
                                                   *tip,
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
    case Action::CREATE_VBK_POP_TX: {
      auto* tip = state.vbk().getBestChain().tip();
      auto tx = mock_miner.createVbkPopTxEndorsingVbkBlock(
          tip->getHeader(), state.btc().getBestChain().tip()->getHash());
      break;
    }

    default:
      break;
  }
}

}  // namespace testing_utils
}  // namespace altintegration
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

void E2EState::createAction(CreateOption create,
                            ForkOption fork,
                            AltBlockTree& tree) {
  ValidationState state;
  switch (create) {
    case CreateOption::CREATE_ALT: {
      // generate new block
      auto& block = *getBlock(fork, tree);
      auto new_block = generateRandomNextBlock(block, tree.getParams());

      break;
    }
    case CreateOption::CREATE_VBK: {
      // generate new block
      auto& block = *getBlock(fork, tree.vbk());
      auto new_block = mock_miner.mineVbkBlocks(1, block, {}, {});
      break;
    }
    case CreateOption::CREATE_BTC: {
      // generate new block
      auto& block = *getBlock(fork, tree.btc());

      std::vector<BtcTx> txs;
      for (const auto& el : this->btc_txs) {
        txs.push_back(el.btc_tx);
      }

      auto new_block = mock_miner.mineBtcBlocks(1, block, txs);

      std::vector<BtcBlockRelation> blocks;
      for (const auto& el : this->btc_txs) {
        blocks.push_back({el, new_block->getHeader()});
      }

      this->btc_blocks.insert(
          this->btc_blocks.begin(), blocks.begin(), blocks.end());
      this->btc_txs.clear();
      break;
    }
    case CreateOption::CREATE_BTC_TX: {
      auto& block = *getBlock(fork, tree.vbk());
      auto tx = mock_miner.createBtcTxEndorsingVbkBlock(block.getHeader());

      this->btc_txs.push_back({tx, block.getHeader()});
      break;
    }
    case CreateOption::CREATE_VBK_TX: {
      auto& block = *getBlock(fork, tree);

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
                                                   tree.getParams());

      auto tx = mock_miner.createVbkTxEndorsingAltBlock(pd);

      this->vbk_txs.push_back(tx);
      break;
    }
    case CreateOption::CREATE_VBK_POP_TX: {
      if (!this->btc_blocks.empty()) {
        auto& block = this->btc_blocks.front();
        auto tx = mock_miner.createVbkPopTxEndorsingVbkBlock(
            block.btc_block,
            block.tx.btc_tx,
            block.tx.endorsed_block,
            tree.btc().getBestChain().tip()->getHash());

        this->vbk_pop_txs.push_back(tx);
        this->btc_blocks.erase(this->btc_blocks.begin());
      }
      break;
    }
    default:
      break;
  }
}

void E2EState::submitAction(SubmitOption submit, MemPool& mempool) {
  ValidationState state;
  switch (submit) {
    case SubmitOption::SUBMIT_VBK: {
      // TODO
      break;
    }
    case SubmitOption::SUBMIT_VTB: {
      // TODO
      break;
    }
    case SubmitOption::SUBMIT_ATV: {
      // TODO
      break;
    }
    default:
      break;
  }
}

}  // namespace testing_utils
}  // namespace altintegration
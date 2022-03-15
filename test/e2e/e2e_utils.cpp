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

AltBlock generateRandomNextBlock(const BlockIndex<AltBlock>& previous,
                                 const AltChainParams&) {
  AltBlock block;
  block.hash = generateRandomBytesVector(32);
  block.height = previous.getHeight() + 1;
  block.previousBlock = previous.getHash();
  block.timestamp = previous.getTimestamp() + 1;
  return block;
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
      alt_blocks.push_back(new_block);

      this->stats.created_alt++;
      break;
    }
    case CreateOption::CREATE_VBK: {
      // generate new block
      auto& block = *getBlock(fork, tree.vbk());
      auto new_block =
          mock_miner.mineVbkBlocks(1, block, this->vbk_txs, this->vbk_pop_txs);

      for (const auto& el : this->vbk_txs) {
        this->vbk_block_tx_rel.push_back({new_block->getHeader(), el});
      }

      for (const auto& el : this->vbk_pop_txs) {
        this->vbk_block_pop_tx_rel.push_back({new_block->getHeader(), el});
      }

      this->vbk_blocks.push_back(new_block->getHeader());

      this->vbk_txs.clear();
      this->vbk_pop_txs.clear();

      this->stats.created_vbk++;
      break;
    }
    case CreateOption::CREATE_BTC: {
      // generate new block
      auto& block = *getBlock(fork, tree.btc());

      std::vector<BtcTx> txs;
      for (const auto& el : this->btc_tx_rel) {
        txs.push_back(el.btc_tx);
      }

      auto new_block = mock_miner.mineBtcBlocks(1, block, txs);

      std::vector<BtcBlockRelation> blocks;
      for (const auto& el : this->btc_tx_rel) {
        blocks.push_back({new_block->getHeader(), el});
      }

      this->btc_block_tx_rel.insert(
          this->btc_block_tx_rel.begin(), blocks.begin(), blocks.end());
      this->btc_tx_rel.clear();

      this->stats.created_btc++;
      break;
    }
    case CreateOption::CREATE_BTC_TX: {
      auto& block = *getBlock(fork, tree.vbk());
      auto tx = mock_miner.createBtcTxEndorsingVbkBlock(block.getHeader());

      this->btc_tx_rel.push_back({tx, block.getHeader()});

      this->stats.created_btc_tx++;
      break;
    }
    case CreateOption::CREATE_VBK_TX: {
      auto& block = *getBlock(fork, tree);

      PublicationData pd =
          GeneratePublicationData(block.getHeader().toRaw(),
                                  block,
                                  {1, 2, 3, 4, 5},
                                  1,
                                  block.getPayloadIds<ATV>(),
                                  block.getPayloadIds<VTB>(),
                                  block.getPayloadIds<VbkBlock>(),
                                  {1, 2, 3, 4, 5},
                                  tree.getParams());

      auto tx = mock_miner.createVbkTxEndorsingAltBlock(pd);

      this->vbk_txs.push_back(tx);

      this->stats.created_vbk_tx++;
      break;
    }
    case CreateOption::CREATE_VBK_POP_TX: {
      if (!this->btc_block_tx_rel.empty()) {
        auto& block = this->btc_block_tx_rel.front();
        auto tx = mock_miner.createVbkPopTxEndorsingVbkBlock(
            block.btc_block,
            block.tx.btc_tx,
            block.tx.endorsed_block,
            tree.btc().getBestChain().tip()->getHash());

        this->vbk_pop_txs.push_back(tx);
        this->btc_block_tx_rel.erase(this->btc_block_tx_rel.begin());

        this->stats.created_vbk_pop_tx++;
      }
      break;
    }
    default:
      break;
  }
}

void E2EState::submitAction(SubmitOption submit,
                            MemPool& mempool,
                            AltBlockTree& tree) {
  ValidationState state;
  switch (submit) {
    case SubmitOption::SUBMIT_VBK: {
      if (!this->vbk_blocks.empty()) {
        auto block = this->vbk_blocks.front();
        mempool.submit(block, state);

        this->vbk_blocks.erase(this->vbk_blocks.begin());
      }
      break;
    }
    case SubmitOption::SUBMIT_VTB: {
      if (!this->vbk_block_pop_tx_rel.empty()) {
        auto rel = this->vbk_block_pop_tx_rel.front();
        auto vtb = mock_miner.createVTB(rel.block, rel.tx);
        mempool.submit(vtb, state);

        this->vbk_block_pop_tx_rel.erase(this->vbk_block_pop_tx_rel.begin());
      }
      break;
    }
    case SubmitOption::SUBMIT_ATV: {
      if (!this->vbk_block_tx_rel.empty()) {
        auto rel = this->vbk_block_tx_rel.front();
        auto atv = mock_miner.createATV(rel.block, rel.tx);
        mempool.submit(atv, state);

        this->vbk_block_tx_rel.erase(this->vbk_block_tx_rel.begin());
      }
      break;
    }
    case SubmitOption::SUBMIT_ALT: {
      if (!this->alt_blocks.empty()) {
        auto block = this->alt_blocks.front();
        auto pop_data = mempool.generatePopData();

        tree.acceptBlockHeader(block, state);
        tree.acceptBlock(block.hash, pop_data);
        tree.comparePopScore(tree.getBestChain().tip()->getHash(), block.hash);

        this->alt_blocks.erase(this->alt_blocks.begin());
      }
      break;
    }
    default:
      break;
  }
}

}  // namespace testing_utils
}  // namespace altintegration
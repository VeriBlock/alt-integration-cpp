// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef TEST_E2E_E2E_UTILS_HPP
#define TEST_E2E_E2E_UTILS_HPP

#include <cstdint>
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/mempool.hpp>
#include <veriblock/pop/mock_miner.hpp>

#include "rand.hpp"

namespace altintegration {
namespace testing_utils {

enum class CreateOption : uint8_t {
  CREATE_ALT = 0,
  CREATE_VBK,
  CREATE_BTC,
  CREATE_BTC_TX,
  CREATE_VBK_TX,
  CREATE_VBK_POP_TX,
  CREATE_VTB,
  CREATE_ATV,

  kMaxValue = CREATE_VBK_POP_TX
};

enum class SubmitOption : uint8_t {
  SUBMIT_ATV = 0,
  SUBMIT_VTB,
  SUBMIT_VBK,
  SUBMIT_ALT,

  kMaxValue = SUBMIT_ALT
};

enum class ForkOption : uint8_t {
  NEXT_AFTER_CURRENT_TIP,
  NEXT_AFTER_ANY_TIP,
  RANDOM_BLOCK,

  kMaxValue = RANDOM_BLOCK
};

template <typename OptionT>
OptionT GetRandomOption() {
  return static_cast<OptionT>(::rand() %
                              static_cast<uint8_t>(OptionT::kMaxValue));
}

template <typename tree_t>
const typename tree_t::index_t* getBlock(ForkOption fork, const tree_t& tree) {
  switch (fork) {
    case ForkOption::NEXT_AFTER_CURRENT_TIP: {
      return tree.getBestChain().tip();
      break;
    }
    case ForkOption::NEXT_AFTER_ANY_TIP: {
      auto tips = tree.getTips();
      return *select_randomly(tips.begin(), tips.end());
      break;
    }
    case ForkOption::RANDOM_BLOCK:
    default: {
      auto blocks = tree.getBlocks();
      return *select_randomly(blocks.begin(), blocks.end());
      break;
    }
  }
}

struct E2EStats {
  uint32_t created_alt{0};
  uint32_t created_vbk{0};
  uint32_t created_btc{0};
  uint32_t created_btc_tx{0};
  uint32_t created_vbk_tx{0};
  uint32_t created_vbk_pop_tx{0};
  uint32_t created_vtb{0};
  uint32_t created_atv{0};
};

struct E2EState {
  E2EState(const AltChainParams& alt_config,
           const VbkChainParams& vbk_config,
           const BtcChainParams& btc_config)
      : mock_miner(alt_config, vbk_config, btc_config) {}

  void createAction(CreateOption create,
                    ForkOption fork,
                    AltBlockTree& current_state);

  void submitAction(SubmitOption submit, MemPool& mempool, AltBlockTree& tree);

  const E2EStats& getStats() const { return this->stats; };

 private:
  struct BtcTxRelation {
    BtcTx btc_tx;
    VbkBlock endorsed_block;
  };

  struct BtcBlockRelation {
    BtcBlock btc_block;
    BtcTxRelation tx;
  };

  struct VbkBlockRelation {
    VbkBlock block;
    VbkTx tx;
  };

  struct VbkBlockPopRelation {
    VbkBlock block;
    VbkPopTx tx;
  };

  MockMiner mock_miner;

  std::vector<VbkTx> vbk_txs;
  std::vector<VbkPopTx> vbk_pop_txs;
  std::vector<BtcTxRelation> btc_tx_rel;
  std::vector<BtcBlockRelation> btc_block_tx_rel;
  std::vector<VbkBlockRelation> vbk_block_tx_rel;
  std::vector<VbkBlockPopRelation> vbk_block_pop_tx_rel;
  std::vector<VbkBlock> vbk_blocks;
  std::vector<AltBlock> alt_blocks;

  E2EStats stats;
};

}  // namespace testing_utils
}  // namespace altintegration

#endif

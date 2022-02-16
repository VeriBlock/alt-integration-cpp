// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef TEST_E2E_E2E_UTILS_HPP
#define TEST_E2E_E2E_UTILS_HPP

#include <cstdint>
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/mock_miner.hpp>

#include "rand.hpp"

namespace altintegration {
namespace testing_utils {

enum class ActionOption : uint8_t {
  MINE_ALT,
  MINE_VBK,
  MINE_BTC,
  CREATE_BTC_TX,
  CREATE_VBK_TX,
  CREATE_VBK_POP_TX,

  kMaxValue = CREATE_VBK_POP_TX
};

enum class ForkOption : uint8_t {
  NEXT_AFTER_CURRENT_TIP,
  NEXT_AFTER_ANY_TIP,
  RANDOM_BLOCK,

  kMaxValue = RANDOM_BLOCK
};

ActionOption GetRandomAction();

ForkOption GetRandomFork();

template <typename block_t, typename params_t>
block_t generateRandomNextBlock(const BlockIndex<block_t>&, const params_t&);

template <>
AltBlock generateRandomNextBlock(const BlockIndex<AltBlock>&,
                                 const AltChainParams&);

template <>
VbkBlock generateRandomNextBlock(const BlockIndex<VbkBlock>&,
                                 const VbkChainParams&);

template <>
BtcBlock generateRandomNextBlock(const BlockIndex<BtcBlock>&,
                                 const BtcChainParams&);

template <typename tree_t>
const typename tree_t::index_t& getBlock(ForkOption fork, const tree_t& tree) {
  switch (fork) {
    case ForkOption::NEXT_AFTER_CURRENT_TIP: {
      return *tree.getBestChain().tip();
      break;
    }
    case ForkOption::NEXT_AFTER_ANY_TIP: {
      auto tips = tree.getTips();
      return *select_randomly(tips.begin(), tips.end());
      break;
    }
    case ForkOption::RANDOM_BLOCK: {
      auto blocks = tree.getBlocks();
      return *select_randomly(blocks.begin(), blocks.end());
      break;
    }
  }
}

struct E2EState {
  MockMiner mock_miner;

  void applyAction(ActionOption action,
                   ForkOption fork,
                   const AltBlockTree& current_state);
};

}  // namespace testing_utils
}  // namespace altintegration

#endif
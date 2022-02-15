// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef TEST_E2E_E2E_UTILS_HPP
#define TEST_E2E_E2E_UTILS_HPP

#include <cstdint>
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/mock_miner.hpp>

namespace altintegration {
namespace testing_utils {

enum class Action : uint8_t {
  MINE_ALT,
  MINE_VBK,
  MINE_BTC,
  CREATE_BTC_TX,
  CREATE_VBK_TX,
  CREATE_VBK_POP_TX,

  kMaxValue = SUBMIT_ALL

};

Action GetRandomAction();

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

struct E2EState {
  MockMiner mock_miner;

  void applyAction(Action action, const AltBlockTree& current_state);
};

}  // namespace testing_utils
}  // namespace altintegration

#endif
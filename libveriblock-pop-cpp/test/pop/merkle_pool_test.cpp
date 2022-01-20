// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/alt-util.hpp>
#include <veriblock/pop/entities/merkle_tree.hpp>
#include <veriblock/pop/mock_miner.hpp>

using namespace altintegration;

struct TlrmMinerCalculationFixture : public ::testing::Test {
  ContextInfoContainer ctx;
  uint256 popdataroot = uint256::fromHex("1337");
  uint256 coinbase = ParseHex("010203040506070809");

  TlrmMinerCalculationFixture() {
    ctx.height = 100;
    ctx.keystones.firstPreviousKeystone = std::vector<uint8_t>{1, 2, 3};
    ctx.keystones.secondPreviousKeystone = std::vector<uint8_t>{3, 2, 1};
  }

  uint256 expectedTlmr() {
    BtcMerkleTree tree({coinbase});
    auto txroot = tree.getMerkleRoot();
    return CalculateTopLevelMerkleRoot(txroot.asVector(), popdataroot, ctx);
  }

  // algorithm which is used for calculation of TLMR on miners given 'steps' -
  // prepared merkle layers
  uint256 merkle_with_first(std::vector<uint256> steps, uint256 cb) {
    std::vector<uint256>::const_iterator i;
    for (i = steps.begin(); i != steps.end(); ++i) {
      cb = sha256twice(cb, *i);
    }

    return cb;
  }
};

TEST_F(TlrmMinerCalculationFixture, Coinbase) {
  // miner builds TLMR given a list of prepared hashes ('steps')
  BtcMerkleTree tree({coinbase});
  std::vector<uint256> steps;
  // we inject 2 more steps
  steps.push_back(popdataroot);
  steps.push_back(ctx.getHash());

  // and get correct TLMR
  auto root = merkle_with_first(steps, tree.getMerkleRoot());
  ASSERT_EQ(root, expectedTlmr());
}

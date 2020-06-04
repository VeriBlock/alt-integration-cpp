// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/adapters/picojson.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <util/pop_test_fixture.hpp>

using namespace altintegration;

TEST(ToJson, BtcBlock) {
  BtcBlock block;
  block.version = 1;
  block.previousBlock = uint256::fromHex("123123123123123");
  block.merkleRoot = uint256::fromHex("abcabc");
  block.bits = 1337;
  block.timestamp = 9999;
  block.nonce = 9379992;

  picojson::value obj(ToJSON<picojson::object>(block));

  std::string expected = R"({
  "bits": 1337,
  "hash": "9025ce6e8008ae58912ebf1003ab80e8ca4f81f6c6dd065c12db47f93cd9722e",
  "merkleRoot": "abcabc0000000000000000000000000000000000000000000000000000000000",
  "nonce": 9379992,
  "previousBlock": "1231231231231200000000000000000000000000000000000000000000000000",
  "timestamp": 9999,
  "version": 1
}
)";

  ASSERT_EQ(obj.serialize(true), expected);
}

TEST(ToJson, VTB) {
  PopTestFixture pop;
  auto tx = pop.popminer->endorseVbkBlock(*pop.popminer->vbk().getBestChain()[0]->header, pop.getLastKnownBtcBlock(), pop.state);
  pop.popminer->vbkmempool.push_back(tx);
  auto block = pop.popminer->mineVbkBlocks(1);
  auto& vtb = pop.popminer->vbkPayloads.at(block->getHash()).at(0);

  picojson::value val(ToJSON<picojson::object>(vtb));
  std::cout << val.serialize(true);
}
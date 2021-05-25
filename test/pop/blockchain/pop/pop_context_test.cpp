// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/config.hpp>
#include <veriblock/pop/mock_miner.hpp>
#include <veriblock/pop/pop_context.hpp>
#include <veriblock/pop/storage/adaptors/inmem_storage_impl.hpp>
#include <veriblock/pop/storage/adaptors/payloads_provider_impl.hpp>

using namespace altintegration;

struct PopContextFixture : public ::testing::Test {
  VbkChainParamsRegTest vbkp;
  BtcChainParamsRegTest btcp;
  adaptors::InmemStorageImpl storage{};
  adaptors::PayloadsStorageImpl payloadsProvider{storage};
  PayloadsIndex payloadsIndex;
  VbkBlockTree local =
      VbkBlockTree(vbkp, btcp, payloadsProvider, payloadsIndex);
  MockMiner remote;

  BlockIndex<BtcBlock>* forkPoint;
  BlockIndex<BtcBlock>* chainAtip;
  BlockIndex<BtcBlock>* chainBtip;
  BlockIndex<VbkBlock>* vbkTip;

  ValidationState state;

  PopContextFixture() {
    EXPECT_TRUE(local.bootstrapWithGenesis(GetRegTestVbkBlock(), state));
    EXPECT_TRUE(local.btc().bootstrapWithGenesis(GetRegTestBtcBlock(), state));
  }

  void SetUp() override {
    // BTC: 100 blocks, then fork A: +50 blocks, fork B: +100 blocks
    forkPoint = remote.mineBtcBlocks(100);
    chainAtip = remote.mineBtcBlocks(50, *forkPoint);
    chainBtip = remote.mineBtcBlocks(100, *forkPoint);

    // mine 100 VBK blocks
    vbkTip = remote.mineVbkBlocks(100);

    // create endorsement of VBKTIP in BTC_A_51, and same endorsement in
    // BTC_B_101
    auto btctx = remote.createBtcTxEndorsingVbkBlock(vbkTip->getHeader());
    // add BTC tx endorsing VBKTIP into next block after chain A tip
    chainAtip = remote.mineBtcBlocks(1, *chainAtip, {btctx});
    // add same btctx to mempool again
    // add BTC tx endorsing VBKTIP into next block after chain B tip
    chainBtip = remote.mineBtcBlocks(1, *chainBtip, {btctx, btctx});

    // create VBK pop tx that has 'block of proof=CHAIN A'
    auto txa = remote.createVbkPopTxEndorsingVbkBlock(chainAtip->getHeader(),
                                                      btctx,
                                                      vbkTip->getHeader(),
                                                      lastKnownLocalBtcBlock());

    // mine txA into VBK 101-th block
    vbkTip = remote.mineVbkBlocks(1, {txa});

    // create VBK pop tx that has 'block of proof=CHAIN B'
    auto txb =
        remote.createVbkPopTxEndorsingVbkBlock(chainBtip->getHeader(),
                                               btctx,
                                               vbkTip->pprev->getHeader(),
                                               lastKnownLocalBtcBlock());

    // mine this tx into 102-th block
    vbkTip = remote.mineVbkBlocks(1, {txb});

    auto vtbsPrev = remote.getVTBs(vbkTip->pprev->getHeader());
    auto vtbs = remote.getVTBs(vbkTip->getHeader());
    // we have 2 distinct VTBs
    ASSERT_EQ(vtbsPrev.size(), 1);
    ASSERT_EQ(vtbs.size(), 1);
  }

  BtcBlock::hash_t lastKnownLocalBtcBlock() {
    auto tip = local.btc().getBestChain().tip();
    EXPECT_TRUE(tip);
    return tip->getHash();
  }

  void makeSureNoDuplicates(const std::vector<uint256>& t) {
    std::unordered_set<uint256> s;
    s.reserve(t.size());
    for (const auto& item : t) {
      auto pair = s.insert(item);
      if (!pair.second) {
        auto index = local.btc().getBlockIndex(*pair.first);
        ASSERT_TRUE(index);
        std::cerr << "Duplicate item: " << index->toPrettyString() << std::endl;
      }
    }

    EXPECT_EQ(s.size(), t.size());
  }
};

TEST_F(PopContextFixture, A) {
  // current REMOTE BTC chain is on chain B
  ASSERT_EQ(remote.btc().getBestChain().tip(), chainBtip);

  // all blocks excluding genesis block
  std::vector<VbkBlock> vbkblocks;
  auto current = vbkTip;
  while (current && current->pprev) {
    vbkblocks.push_back(current->getHeader());
    current = current->pprev;
  }
  std::reverse(vbkblocks.begin(), vbkblocks.end());

  // apply remote vbk blocks to current local vbk tip
  for (const auto& b : vbkblocks) {
    ASSERT_TRUE(local.acceptBlockHeader(b, state)) << state.GetPath();
  }

  // local and remote VBK chains are same
  ASSERT_EQ(local.getBestChain().tip()->getHash(),
            remote.vbk().getBestChain().tip()->getHash());
  // local and remote BTC chains are different
  ASSERT_NE(local.btc().getBestChain().tip()->getHash(),
            remote.btc().getBestChain().tip()->getHash());

  auto acceptAllVtbsFromVBKblock = [&](const BlockIndex<VbkBlock>* containing) {
    auto vtbs = remote.getVTBs(containing->getHeader());

    ASSERT_TRUE(local.acceptBlockHeader(containing->getHeader(), state));
    PopData pd;
    pd.vtbs = vtbs;
    payloadsProvider.writePayloads(pd);
    ASSERT_TRUE(
        local.addPayloads(containing->getHeader().getHash(), {vtbs}, state))
        << state.toString();
  };

  // and now accept VBK tip again, with VTBs
  acceptAllVtbsFromVBKblock(vbkTip);

  // and now our local BTC tree must know all blocks from active chain B
  ASSERT_EQ(local.btc().getBestChain().tip()->getHash(),
            remote.btc().getBestChain().tip()->getHash());

  // local BTC tree must NOT know about blocks from chain A
  ASSERT_FALSE(local.btc().getBlockIndex(chainAtip->getHash()));

  // first containing context block is BTC block #1

  // now we add VTB from btcA
  acceptAllVtbsFromVBKblock(vbkTip->pprev);

  auto* localVbkTip = local.getBlockIndex(vbkTip->getHash());
  ASSERT_TRUE(local.setState(localVbkTip->getHash(), state));

  // our local BTC is still same as remote
  ASSERT_EQ(local.btc().getBestChain().tip()->getHash(),
            remote.btc().getBestChain().tip()->getHash());

  // but now we know blocks from chain A
  ASSERT_TRUE(local.btc().getBlockIndex(chainAtip->getHash()))
      << state.toString();
}

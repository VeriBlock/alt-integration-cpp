#ifndef ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP
#define ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP

#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/mock_miner.hpp>

namespace altintegration {

struct TestBlockchainFixture {
  std::shared_ptr<BtcChainParams> btcparam =
      std::make_shared<BtcChainParamsRegTest>();
  std::shared_ptr<VbkChainParams> vbkparam =
      std::make_shared<VbkChainParamsRegTest>();

  // miners
  std::shared_ptr<MockMiner> popminer = std::make_shared<MockMiner>();
  std::shared_ptr<Miner<BtcBlock, BtcChainParams>> btcpowminer =
      std::make_shared<Miner<BtcBlock, BtcChainParams>>(*btcparam);
  std::shared_ptr<Miner<VbkBlock, VbkChainParams>> vbkpowminer =
      std::make_shared<Miner<VbkBlock, VbkChainParams>>(*vbkparam);

  // repos
  std::shared_ptr<PayloadsRepository<VTB>> vtbrepo =
      std::make_shared<PayloadsRepositoryInmem<VTB>>();

  // trees
  VbkBlockTree vbktree = VbkBlockTree(
      *vbkparam,
      VbkBlockTree::PopForkComparator{*vtbrepo, *btcparam, *vbkparam});
  BlockTree<BtcBlock, BtcChainParams>& btctree = vbktree.btc();

  // pending endorsing transactions
  std::vector<VTB> vtbpool;
  std::vector<ATV> atvpool;

  ValidationState state;

  TestBlockchainFixture() {
    EXPECT_TRUE(popminer->btc().bootstrapWithGenesis(state));
    EXPECT_TRUE(popminer->vbk().bootstrapWithGenesis(state));
    EXPECT_TRUE(vbktree.bootstrapWithGenesis(state));
    EXPECT_TRUE(vbktree.btc().bootstrapWithGenesis(state));
  }

  void endorseVBK(uint32_t height) {
    auto& vbkbest = vbktree.getBestChain();
    assert(vbkbest.tip() && "VBK blockchain is not bootstrapped");

    auto* endorsed = vbkbest[height];
    ASSERT_TRUE(endorsed) << "can not find endorsed block at height " << height;
    endorseVBK(*endorsed);
  }

  void endorseVBK(const BlockIndex<VbkBlock>& index) {
    auto& btcbest = vbktree.btc().getBestChain();
    assert(btcbest.tip() && "BTC blockchain is not bootstrapped");

    VTB vtb =
        popminer->generateVTB(index.header, btcbest.tip()->getHash(), state);
    vtbpool.push_back(vtb);
  }

  void endorseVBK(const VbkBlock::hash_t& hash) {
    auto* endorsed = vbktree.getBlockIndex(hash);
    ASSERT_TRUE(endorsed) << "can not find endorsed block with hash " << hash.toHex();
    return endorseVBK(*endorsed);
  }

//  void mineVbkBlocks(size_t amount) {
//
//  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_TEST_BLOCKCHAIN_FIXTURE_HPP

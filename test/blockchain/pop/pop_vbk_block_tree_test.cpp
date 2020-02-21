#include <gtest/gtest.h>

#include <memory>

#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/storage/block_repository_inmem.hpp"

using namespace VeriBlock;

struct VbkBlockTreeTest : public VbkBlockTree {
  ~VbkBlockTreeTest() override = default;

  VbkBlockTreeTest(
      BtcTree& btc,
      std::shared_ptr<BlockRepository<BlockIndex<VbkBlock>>> blockrepo,
      std::shared_ptr<VbkChainParams> params)
      : VbkBlockTree(btc, nullptr, blockrepo, params) {}

  std::vector<ProtoKeystoneContext> getProtoKeystoneContextTest(
      const Chain<VbkBlock>& chain) {
    return getProtoKeystoneContext(chain);
  }
};

struct VbkBlockTreeTestFixture : ::testing::Test {
  std::shared_ptr<VbkBlockTreeTest> vbkTest;

  std::shared_ptr<BlockRepository<BlockIndex<BtcBlock>>> btc_repo;
  std::shared_ptr<BtcChainParams> btc_params;
  std::shared_ptr<BlockTree<BtcBlock, BtcChainParams>> btcTree;

  std::shared_ptr<BlockRepository<BlockIndex<VbkBlock>>> vbk_repo;
  std::shared_ptr<VbkChainParams> vbk_params;

  VbkBlockTreeTestFixture() {
    btc_repo = std::make_shared<BlockRepositoryInmem<BlockIndex<BtcBlock>>>();
    btc_params = std::make_shared<BtcChainParamsRegTest>();
    btcTree = std::make_shared<BlockTree<BtcBlock, BtcChainParams>>(btc_repo,
                                                                    btc_params);

    vbk_repo = std::make_shared<BlockRepositoryInmem<BlockIndex<VbkBlock>>>();
    vbk_params = std::make_shared<VbkChainParamsRegTest>();

    vbkTest =
        std::make_shared<VbkBlockTreeTest>(*btcTree, vbk_repo, vbk_params);
  }
};

TEST_F(VbkBlockTreeTestFixture, getProtoKeystoneContext_test) {}

// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/entities/merkle_tree.hpp>
#include <veriblock/pop/stateless_validation.hpp>

#include <veriblock/pop/literals.hpp>

using namespace altintegration;

struct MerkleTreeOnTxTest : public ::testing::TestWithParam<int> {
  int32_t treeIndex = 0;  // only POP
  std::vector<VbkPopTx::hash_t> txes;
  std::shared_ptr<VbkMerkleTree> mtree;

  void setNtxes(size_t n) {
    size_t i = 0;
    txes.clear();
    std::generate_n(
        std::back_inserter(txes), n, [&]() { return ArithUint256(i++); });
    mtree = std::make_shared<VbkMerkleTree>(txes, treeIndex);
  }

  VbkMerklePath makePath(int n) {
    VbkMerklePath path;
    path.layers = mtree->getMerklePathLayers(n);
    path.subject = txes[n];
    path.treeIndex = treeIndex;
    path.index = n;
    return path;
  }

  bool checkPath(const VbkMerklePath& p, int n) {
    auto root = mtree->getMerkleRoot().trim<VBK_MERKLE_ROOT_HASH_SIZE>();
    return checkMerklePath(p, txes[n], root, state);
  }

  void test(size_t n) {
    setNtxes(n);
    for (size_t i = 0; i < n; i++) {
      VbkMerklePath p = makePath((int)i);
      for (size_t j = 0; j < n; j++) {
        bool ret = checkPath(p, (int32_t)j);
        if (i == j) {
          EXPECT_TRUE(ret) << state.GetDebugMessage();
        } else {
          EXPECT_FALSE(ret) << state.GetDebugMessage();
        }
      }
    }
  }

  ValidationState state;
};

TEST_P(MerkleTreeOnTxTest, CheckForFixedTreeSize) { test((size_t)GetParam()); }

INSTANTIATE_TEST_SUITE_P(MerklePath,
                         MerkleTreeOnTxTest,
                         testing::Range(1, 100));

TEST(MerklePath, RegressionWhenPopTx) {
  VbkMerklePath path;
  path.treeIndex = 0;
  path.index = 13;
  path.subject =
      "2A014E88ED7AB65CDFAA85DAEAB07EEA6CBA5E147F736EDD8D02C2F9DDF0DEC6"_unhex;
  path.layers = {
      "5B977EA09A554AD56957F662284044E7D37450DDADF7DB3647712F5969399787"_unhex,
      "20D0A3D873EEEEE6A222A75316DCE60B53CA43EAEA09D27F0ECE897303A53AE9"_unhex,
      "C06FE913DCA5DC2736563B80834D69E6DFDF1B1E92383EA62791E410421B6C11"_unhex,
      "049F68D350EEB8B3DF630C8308B5C8C2BA4CD6210868395B084AF84D19FF0E90"_unhex,
      "0000000000000000000000000000000000000000000000000000000000000000"_unhex,
      "36252DFC621DE420FB083AD9D8767CBA627EDDEEC64E421E9576CEE21297DD0A"_unhex};

  uint256 expected =
      "B53C1F4E259E6A0DF23721A0B3B4B7ABD730436A85A02A72B9DD6291507A7D61"_unhex;

  EXPECT_EQ(expected.trim<VBK_MERKLE_ROOT_HASH_SIZE>(),
            path.calculateMerkleRoot());
}

TEST(MerklePath, RegressionWhenNormalTx) {
  VbkMerklePath path;
  path.treeIndex = 1;
  path.index = 13;
  path.subject =
      "2A014E88ED7AB65CDFAA85DAEAB07EEA6CBA5E147F736EDD8D02C2F9DDF0DEC6"_unhex;
  path.layers = {
      "5B977EA09A554AD56957F662284044E7D37450DDADF7DB3647712F5969399787"_unhex,
      "20D0A3D873EEEEE6A222A75316DCE60B53CA43EAEA09D27F0ECE897303A53AE9"_unhex,
      "C06FE913DCA5DC2736563B80834D69E6DFDF1B1E92383EA62791E410421B6C11"_unhex,
      "049F68D350EEB8B3DF630C8308B5C8C2BA4CD6210868395B084AF84D19FF0E90"_unhex,
      "0000000000000000000000000000000000000000000000000000000000000000"_unhex,
      "36252DFC621DE420FB083AD9D8767CBA627EDDEEC64E421E9576CEE21297DD0A"_unhex};

  uint256 expected =
      "44450212738058A9B3119472D3ADC849C2B719FF8AD1E8C0EB5003554756F1E4"_unhex;

  EXPECT_EQ(expected.trim<VBK_MERKLE_ROOT_HASH_SIZE>(),
            path.calculateMerkleRoot());
}

template <typename T>
struct GetMerkleRootTest : public ::testing::Test {};

TYPED_TEST_SUITE_P(GetMerkleRootTest);

TYPED_TEST_P(GetMerkleRootTest, CalculateRoot_1) {
  std::vector<uint256> txes{uint256::fromHex("01")};

  TypeParam tree(txes);
  auto expected = txes[0].reverse();
  ASSERT_EQ(tree.getMerkleRoot(), expected);
}

TYPED_TEST_P(GetMerkleRootTest, CalculateRoot_2) {
  std::vector<uint256> txes{
      uint256::fromHex("01"),
      uint256::fromHex("02"),
  };

  uint256 expected = sha256twice(txes[0], txes[1]).reverse();

  TypeParam tree(txes);
  ASSERT_EQ(tree.getMerkleRoot(), expected);
}

TYPED_TEST_P(GetMerkleRootTest, CalculateRoot_3) {
  std::vector<uint256> txes{
      uint256::fromHex("01"),
      uint256::fromHex("02"),
      uint256::fromHex("03"),
  };

  uint256 A = sha256twice(txes[0], txes[1]);
  uint256 B = sha256twice(txes[2], txes[2]);
  uint256 expected = sha256twice(A, B).reverse();

  TypeParam tree(txes);
  ASSERT_EQ(tree.getMerkleRoot(), expected);
}

REGISTER_TYPED_TEST_SUITE_P(GetMerkleRootTest,
                            CalculateRoot_1,
                            CalculateRoot_2,
                            CalculateRoot_3);

typedef testing::
    Types<BtcMerkleTree, PayloadsMerkleTree<ATV>, PayloadsMerkleTree<VTB>>
        TreesUnderTest;
INSTANTIATE_TYPED_TEST_SUITE_P(MTree, GetMerkleRootTest, TreesUnderTest);
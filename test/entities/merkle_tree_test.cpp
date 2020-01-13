#include <gtest/gtest.h>

#include <veriblock/entities/merkle_tree.hpp>

using namespace VeriBlock;

struct TestHasher {
  ByteArray hash(ByteArray bytes) { return bytes; }
  ByteArray hash(ByteArray bytes1, ByteArray bytes2) {
    ByteArray result(bytes1.size());
    for (int i = 0; i < bytes1.size(); ++i) {
      result[i] = bytes1[i] + bytes2[i];
    }
    return result;
  }
};

struct TestMerkleTree : public MerkleTree<TestHasher> {
  int testGetBottomWidth(const int& n) { return getBottomWidth(n); }

  std::vector<ByteArray> testInitBottomLayer(
      const std::vector<ByteArray>& transactions) {
    vHash.clear();
    initBottomLayer(transactions);
    return vHash;
  }

  std::vector<ByteArray> testBuildTree(
      const std::vector<ByteArray>& transactions) {
    vHash.clear();
    buildTree(transactions);
    return vHash;
  }
};

TEST(MerkleTree, getBottomWidth_test) {
  TestMerkleTree testMerkleTree;

  EXPECT_EQ(8, testMerkleTree.testGetBottomWidth(8));
  EXPECT_EQ(16, testMerkleTree.testGetBottomWidth(16));
  EXPECT_EQ(8, testMerkleTree.testGetBottomWidth(5));
  EXPECT_EQ(8, testMerkleTree.testGetBottomWidth(7));
  EXPECT_EQ(8, testMerkleTree.testGetBottomWidth(7));
  EXPECT_EQ(16, testMerkleTree.testGetBottomWidth(15));
  EXPECT_EQ(16, testMerkleTree.testGetBottomWidth(11));
  EXPECT_EQ(32, testMerkleTree.testGetBottomWidth(22));
  EXPECT_EQ(32, testMerkleTree.testGetBottomWidth(25));
}

TEST(MerkleTree, initBottomLayer_test) {
  TestMerkleTree testMerkleTree;

  std::vector<ByteArray> transactions;
  transactions.push_back(ByteArray(2, 1));
  transactions.push_back(ByteArray(2, 2));
  transactions.push_back(ByteArray(2, 3));
  transactions.push_back(ByteArray(2, 4));
  transactions.push_back(ByteArray(2, 5));

  std::vector<ByteArray> expectedValue;
  expectedValue.push_back(ByteArray(2, 1));
  expectedValue.push_back(ByteArray(2, 2));
  expectedValue.push_back(ByteArray(2, 3));
  expectedValue.push_back(ByteArray(2, 4));
  expectedValue.push_back(ByteArray(2, 5));
  expectedValue.push_back(ByteArray(2, 5));
  expectedValue.push_back(ByteArray(2, 5));
  expectedValue.push_back(ByteArray(2, 5));

  EXPECT_EQ(expectedValue, testMerkleTree.testInitBottomLayer(transactions));

  transactions.clear();
  transactions.push_back(ByteArray(2, 1));
  transactions.push_back(ByteArray(2, 2));
  transactions.push_back(ByteArray(2, 3));
  transactions.push_back(ByteArray(2, 4));
  transactions.push_back(ByteArray(2, 5));
  transactions.push_back(ByteArray(2, 6));
  transactions.push_back(ByteArray(2, 7));
  transactions.push_back(ByteArray(2, 8));
  transactions.push_back(ByteArray(2, 9));
  transactions.push_back(ByteArray(2, 10));

  expectedValue.clear();
  expectedValue.push_back(ByteArray(2, 1));
  expectedValue.push_back(ByteArray(2, 2));
  expectedValue.push_back(ByteArray(2, 3));
  expectedValue.push_back(ByteArray(2, 4));
  expectedValue.push_back(ByteArray(2, 5));
  expectedValue.push_back(ByteArray(2, 6));
  expectedValue.push_back(ByteArray(2, 7));
  expectedValue.push_back(ByteArray(2, 8));
  expectedValue.push_back(ByteArray(2, 9));
  expectedValue.push_back(ByteArray(2, 10));
  expectedValue.push_back(ByteArray(2, 9));
  expectedValue.push_back(ByteArray(2, 10));
  expectedValue.push_back(ByteArray(2, 9));
  expectedValue.push_back(ByteArray(2, 10));
  expectedValue.push_back(ByteArray(2, 9));
  expectedValue.push_back(ByteArray(2, 10));

  EXPECT_EQ(expectedValue, testMerkleTree.testInitBottomLayer(transactions));
}

TEST(MerkleTree, buildTree_test) {
  TestMerkleTree testMerkleTree;

  std::vector<ByteArray> transactions;
  transactions.push_back(ByteArray(2, 1));
  transactions.push_back(ByteArray(2, 2));
  transactions.push_back(ByteArray(2, 3));
  transactions.push_back(ByteArray(2, 4));
  transactions.push_back(ByteArray(2, 5));

  std::vector<ByteArray> expectedValue;
  expectedValue.push_back(ByteArray(2, 1));
  expectedValue.push_back(ByteArray(2, 2));
  expectedValue.push_back(ByteArray(2, 3));
  expectedValue.push_back(ByteArray(2, 4));
  expectedValue.push_back(ByteArray(2, 5));
  expectedValue.push_back(ByteArray(2, 5));
  expectedValue.push_back(ByteArray(2, 5));
  expectedValue.push_back(ByteArray(2, 5));

  expectedValue.push_back(ByteArray(2, 3));
  expectedValue.push_back(ByteArray(2, 7));
  expectedValue.push_back(ByteArray(2, 10));
  expectedValue.push_back(ByteArray(2, 10));

  expectedValue.push_back(ByteArray(2, 10));
  expectedValue.push_back(ByteArray(2, 20));

  expectedValue.push_back(ByteArray(2, 30));

  EXPECT_EQ(expectedValue, testMerkleTree.testBuildTree(transactions));
  // EXPECT_EQ(ByteArray(2, 30), testMerkleTree.getMerkleRoot());
}

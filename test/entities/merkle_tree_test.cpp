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
  TestMerkleTree() {}

  TestMerkleTree(const std::vector<ByteArray>& transactions)
      : MerkleTree(transactions) {}

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
  EXPECT_EQ(ByteArray(2, 30), testMerkleTree.getMerkleRoot());
}

TEST(MerkleTree, getMerklePath_test) {
 
  //             30
  //      /               \  
  //     10               20 
  //  /       \        /      \ 
  //  3       7       10      10
  // / \    /   \   /   \   /   \
  // 1  2   3   4   5   5   5   5 

  std::vector<ByteArray> transactions;
  transactions.push_back(ByteArray(2, 1));
  transactions.push_back(ByteArray(2, 2));
  transactions.push_back(ByteArray(2, 3));
  transactions.push_back(ByteArray(2, 4));
  transactions.push_back(ByteArray(2, 5));

  TestMerkleTree testMerkleTree(transactions);

  std::vector<ByteArray> merklePath =
      testMerkleTree.getMerklePath(transactions[0]);

  EXPECT_EQ(4, merklePath.size());
  EXPECT_EQ(transactions[0], merklePath[0]);
  EXPECT_EQ(transactions[1], merklePath[1]);
  EXPECT_EQ(ByteArray(2, 7), merklePath[2]);
  EXPECT_EQ(ByteArray(2, 20), merklePath[3]);

  //                                                             408
  //                                /                                                              \
  //                              136                                                             272
  //               /                                \                                /                            \          
  //              36                                100                            136                            136
  //      /               \                 /               \                /             \                /              \
  //     10               26              42                58              68             68              68              68
  //  /       \        /      \        /      \         /      \         /     \        /      \        /      \        /      \
  //  3       7       11      15      19      23       27      31      34      34      34      34      34      34      34      34    
  // / \    /   \   /   \   /   \   /   \    / \      /  \    /  \    /  \    /  \    /  \    /  \    /  \    /  \    /  \    /  \
  // 1  2   3   4   5   6   7   8   9   10  11  12   13  14  15  16  17  17  17  17  17  17  17  17  17  17  17  17  17  17  17  17

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
  transactions.push_back(ByteArray(2, 11));
  transactions.push_back(ByteArray(2, 12));
  transactions.push_back(ByteArray(2, 13));
  transactions.push_back(ByteArray(2, 14));
  transactions.push_back(ByteArray(2, 15));
  transactions.push_back(ByteArray(2, 16));
  transactions.push_back(ByteArray(2, 17));

  testMerkleTree = TestMerkleTree(transactions);

  merklePath = testMerkleTree.getMerklePath(transactions[2]);

  EXPECT_EQ(6, merklePath.size());
  EXPECT_EQ(transactions[2], merklePath[0]);
  EXPECT_EQ(transactions[3], merklePath[1]);
  EXPECT_EQ(ByteArray(2, 3), merklePath[2]);
  EXPECT_EQ(ByteArray(2, 26), merklePath[3]);
  EXPECT_EQ(ByteArray(2, 100), merklePath[4]);
  int value = 272;
  EXPECT_EQ(ByteArray(2, static_cast<uint8_t>(value)), merklePath[5]);
}

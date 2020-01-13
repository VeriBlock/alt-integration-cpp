#ifndef VERIBLOCK_MERKLE_TREE_H_
#define VERIBLOCK_MERKLE_TREE_H_

#include <stdint.h>

#include <vector>

namespace VeriBlock {
using ByteArray = std::vector<uint8_t>;

template <typename Hasher>
class MerkleTree {
 protected:
  Hasher hashWriter;
  uint32_t nTransactions;
  std::vector<ByteArray> vHash;

 protected:
  uint32_t getBottomWidth(const size_t& n) {
    int width = 1;
    while (n > width) {
      width <<= 1;
    }
    return width;
  }

  void initBottomLayer(const std::vector<ByteArray>& transactions) {
    uint32_t bottomWidth = getBottomWidth(transactions.size());

    for (uint32_t i = 0; vHash.size() != bottomWidth; ++i) {
      if (i < transactions.size()) {
        vHash.push_back(hashWriter.hash(transactions[i]));
      } else if (vHash.size() & 1) {
        vHash.push_back(vHash.back());
      } else {
        vHash.push_back(*--(--vHash.end()));
        vHash.push_back(*--(--vHash.end()));
      }
    }
  }

  void buildTree(const std::vector<ByteArray>& transactions) {
    initBottomLayer(transactions);
    size_t bottomWidth = vHash.size();
    size_t j = 0;
    while (bottomWidth != 0) {
      for (int i = 0; i + 1 < bottomWidth; i += 2) {
        vHash.push_back(hashWriter.hash(vHash[j + i], vHash[j + i + 1]));
      }
      j += bottomWidth;
      bottomWidth >>= 1;
    }
  }

 public:
  MerkleTree() : nTransactions(0) {}

  MerkleTree(std::vector<ByteArray> transactions)
      : nTransactions(transactions.size()) {}

  ByteArray getMerkleRoot() { return vHash.back(); }
};

}  // namespace VeriBlock
#endif  // !VERIBLOCK_MERKLE_TREE_H_

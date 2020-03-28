#ifndef VERIBLOCK_MERKLE_TREE_H_
#define VERIBLOCK_MERKLE_TREE_H_

#include <stdint.h>

#include <algorithm>
#include <vector>
#include <veriblock/entities/merkle_path.hpp>

namespace VeriBlock {
using ByteArray = std::vector<uint8_t>;

template <typename Hasher>
class MerkleTree {
 protected:
  Hasher hashWriter;
  size_t nTransactions;
  std::vector<ByteArray> vHash;

 protected:
  uint32_t getBottomWidth(const size_t& n) {
    size_t width = 1;
    while (n > width) {
      width <<= 1u;
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
      for (size_t i = 0; i + 1 < bottomWidth; i += 2) {
        vHash.push_back(hashWriter.hash(vHash[j + i], vHash[j + i + 1]));
      }
      j += bottomWidth;
      bottomWidth >>= 1u;
    }
  }

 public:
  MerkleTree() : nTransactions(0) {}

  MerkleTree(const std::vector<ByteArray>& transactions)
      : nTransactions(transactions.size()) {
    buildTree(transactions);
  }

  std::vector<ByteArray> getMerklePath(const ByteArray& transaction) {
    ByteArray hash = hashWriter.hash(transaction);
    uint32_t bottomWidth = getBottomWidth(nTransactions);

    std::vector<ByteArray> merklePath;

    size_t index = 0;
    for (size_t i = 0; i < bottomWidth; ++i) {
      if (vHash[i] == hash) {
        index = i;
        merklePath.push_back(hash);
        break;
      }
    }
    while (index != vHash.size() - 1)
      if (index & 1u) {
        merklePath.push_back(vHash[index - 1]);
        index = index / 2 + bottomWidth;
      } else {
        merklePath.push_back(vHash[index + 1]);
        index = (index + 1) / 2 + bottomWidth;
      }
    return merklePath;
  }

  std::vector<ByteArray> getMerklePath1(const ByteArray& transaction) {
    ByteArray hash = hashWriter.hash(transaction);
    uint32_t bottomWidth = getBottomWidth(nTransactions);

    std::vector<ByteArray> merklePath;

    size_t index = 0;
    for (size_t i = 0; i < bottomWidth; ++i) {
      if (vHash[i] == hash) {
        index = i;
        merklePath.push_back(hash);
        break;
      }
    }
    while (index != vHash.size() - 1)
      if (index & 1u) {
        merklePath.push_back(vHash[index - 1]);
        index = index / 2 + bottomWidth;
      } else {
        merklePath.push_back(vHash[index + 1]);
        index = (index + 1) / 2 + bottomWidth;
      }
    return merklePath;
  }

  ByteArray getMerkleRoot() { return vHash.back(); }
};

}  // namespace VeriBlock
#endif  // !VERIBLOCK_MERKLE_TREE_H_

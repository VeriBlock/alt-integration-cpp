// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_MERKLE_TREE_H_
#define VERIBLOCK_MERKLE_TREE_H_

#include <algorithm>
#include <deque>
#include <vector>
#include <veriblock/arith_uint256.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/merkle_path.hpp>

namespace altintegration {

//! @private
template <typename Specific, typename TxHash>
struct MerkleTree {
  using hash_t = TxHash;

  MerkleTree(Specific& instance, const std::vector<hash_t>& transactions)
      : instance(instance) {
    buildTree(transactions);
  }

  std::vector<hash_t> getMerklePathLayers(const hash_t& hash) const {
    VBK_ASSERT(!layers.empty());
    auto& leafs = layers[0];
    auto it = std::find(leafs.begin(), leafs.end(), hash);
    VBK_ASSERT_MSG(it != leafs.end(),
                   "can not find transaction in merkle tree");

    if (leafs.size() == 1) {
      // no layers
      return {};
    }

    size_t index = std::distance(leafs.begin(), it);
    VBK_ASSERT(index < leafs.size());
    std::vector<hash_t> merklePath{};
    for (size_t i = 0; i < layers.size() - 1; i++) {
      auto& layer = layers[i];
      if (index % 2 == 0) {
        if (layer.size() == index + 1) {
          merklePath.push_back(layer[index]);
        } else {
          merklePath.push_back(layer[index + 1]);
        }
      } else {
        merklePath.push_back(layer[index - 1]);
      }

      index /= 2;
    }

    return instance.finalizePath(std::move(merklePath));
  }

  hash_t getMerkleRoot() { return instance.finalizeRoot(); }

 protected:
  void buildTree(std::vector<hash_t> layer) {
    size_t n = layer.size();
    if (n == 0) {
      return;
    }
    if (n == 1) {
      layers = {layer};
      return;
    }

    layers.push_back(layer);

    layer.resize(n % 2 == 0 ? n : n + 1);
    while (n > 1) {
      if (n % 2) {
        layer[n] = layer[n - 1];
        ++n;
      }
      n /= 2;
      for (size_t i = 0; i < n; i++) {
        layer[i] = instance.hash(layer[2 * i], layer[2 * i + 1]);
      }
      layers.push_back({layer.begin(), layer.begin() + n});
    }
  }

 protected:
  Specific& instance;
  std::vector<std::vector<hash_t>> layers;
};

//! @private
struct VbkMerkleTree : public MerkleTree<VbkMerkleTree, uint256> {
  using base = MerkleTree<VbkMerkleTree, uint256>;

  explicit VbkMerkleTree(const std::vector<hash_t>& txes, int treeIndex)
      : base(*this, txes), treeIndex(treeIndex) {}

  hash_t hash(const hash_t& a, const hash_t& b) { return sha256(a, b); }

  std::vector<hash_t> finalizePath(std::vector<hash_t> path) {
    // opposite tree merkle root (we don't have the opposite tree)
    path.emplace_back();

    // metapackage hash
    path.emplace_back();
    return path;
  }

  hash_t finalizeRoot() {
    if (layers.size() == 1) {
      // the only layer
      VBK_ASSERT(layers[0].size() == 1);
      return layers[0][0];
    }

    auto& normalMerkleRoot = layers.back()[0];
    auto cursor = hash_t();
    VBK_ASSERT_MSG(treeIndex >= 0 && treeIndex <= 1,
                   "tree index can be either 0 or 1");
    if (treeIndex == 0) {
      // POP TXes: zeroes are on the left subtree
      cursor = hash(normalMerkleRoot, cursor);
    } else if (treeIndex == 1) {
      // NORMAL TXes: zeroes are on the right subtree
      cursor = hash(cursor, normalMerkleRoot);
    }

    // add metapackage hash (also all zeroes) to the left subtree
    auto metapackageHash = hash_t();
    return hash(metapackageHash, cursor);
  }

 private:
  int treeIndex = 0;
};

//! @private
struct BtcMerkleTree : public MerkleTree<BtcMerkleTree, uint256> {
  using base = MerkleTree<BtcMerkleTree, uint256>;

  explicit BtcMerkleTree(const std::vector<hash_t>& txes) : base(*this, txes) {}

  hash_t hash(const hash_t& a, const hash_t& b) { return sha256twice(a, b); }

  std::vector<hash_t> finalizePath(const std::vector<hash_t>& path) {
    return path;
  }

  hash_t finalizeRoot() {
    VBK_ASSERT(!layers.empty());
    return layers[0][0].reverse();
  }
};

//! @private
template <typename pop_t>
struct PayloadsMerkleTree
    : public MerkleTree<PayloadsMerkleTree<pop_t>, typename pop_t::id_t> {
  using base = MerkleTree<PayloadsMerkleTree<pop_t>, typename pop_t::id_t>;
  using hash_t = typename base::hash_t;

  explicit PayloadsMerkleTree(const std::vector<hash_t>& hashes)
      : base(*this, hashes) {}

  hash_t hash(const hash_t& a, const hash_t& b) {
    return sha256twice(a, b).template trimLE<hash_t::size()>();
  }

  std::vector<hash_t> finalizePath(const std::vector<hash_t>& path) {
    return path;
  }

  hash_t finalizeRoot() {
    if (this->layers.empty()) {
      return hash_t();
    }
    return this->layers[0][0].reverse();
  }
};

}  // namespace altintegration
#endif  // !VERIBLOCK_MERKLE_TREE_H_

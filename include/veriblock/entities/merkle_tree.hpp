// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_MERKLE_TREE_H_
#define VERIBLOCK_MERKLE_TREE_H_

#include <algorithm>
#include <deque>
#include <vector>
#include <veriblock/arith_uint256.hpp>
#include <veriblock/entities/merkle_path.hpp>

namespace altintegration {

template <typename Specific>
struct MerkleTree {
  using txhash_t = uint256;

  MerkleTree(Specific& instance, const std::vector<txhash_t>& transactions)
      : instance(instance) {
    buildTree(transactions);
  }

  std::vector<txhash_t> getMerklePathLayers(const txhash_t& hash) {
    assert(!layers.empty());
    auto& leafs = layers[0];
    auto it = std::find(leafs.begin(), leafs.end(), hash);
    if (it == leafs.end()) {
      throw std::invalid_argument("can not find transaction in merkle tree");
    }

    if (leafs.size() == 1) {
      // no layers
      return {};
    }

    size_t index = std::distance(leafs.begin(), it);
    assert(index < leafs.size());
    std::vector<txhash_t> merklePath{};
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

  txhash_t getMerkleRoot() { return instance.finalizeRoot(); }

 protected:
  void buildTree(std::vector<txhash_t> layer) {
    size_t n = layer.size();
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
  std::vector<std::vector<txhash_t>> layers;
};

struct VbkMerkleTree : public MerkleTree<VbkMerkleTree> {
  using base = MerkleTree<VbkMerkleTree>;

  explicit VbkMerkleTree(const std::vector<txhash_t>& txes, int treeIndex)
      : base(*this, txes), treeIndex(treeIndex) {}

  txhash_t hash(const txhash_t& a, const txhash_t& b) { return sha256(a, b); }

  std::vector<txhash_t> finalizePath(std::vector<txhash_t> path) {
    // opposite tree merkle root (we don't have the opposite tree)
    path.emplace_back();

    // metapackage hash
    path.emplace_back();
    return path;
  }

  txhash_t finalizeRoot() {
    if (layers.size() == 1) {
      // the only layer
      assert(layers[0].size() == 1);
      return layers[0][0];
    }

    auto& normalMerkleRoot = layers.back()[0];
    auto cursor = txhash_t();
    if (treeIndex == 0) {
      // POP TXes: zeroes are on the left subtree
      cursor = hash(normalMerkleRoot, cursor);
    } else if (treeIndex == 1) {
      // NORMAL TXes: zeroes are on the right subtree
      cursor = hash(cursor, normalMerkleRoot);
    } else {
      throw std::invalid_argument("treeIndex can be either 0 or 1");
    }

    // add metapackage hash (also all zeroes) to the left subtree
    auto metapackageHash = uint256();
    return hash(metapackageHash, cursor);
  }

 private:
  int treeIndex = 0;
};

struct BtcMerkleTree : public MerkleTree<BtcMerkleTree> {
  using base = MerkleTree<BtcMerkleTree>;

  explicit BtcMerkleTree(const std::vector<txhash_t>& txes)
      : base(*this, txes) {}

  txhash_t hash(const txhash_t& a, const txhash_t& b) {
    return sha256twice(a, b);
  }

  std::vector<txhash_t> finalizePath(const std::vector<txhash_t>& path) {
    return path;
  }

  uint256 finalizeRoot() {
    assert(!layers.empty());
    return layers.back()[0].reverse();
  }
};

}  // namespace altintegration
#endif  // !VERIBLOCK_MERKLE_TREE_H_

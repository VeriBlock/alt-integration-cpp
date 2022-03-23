// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_MERKLE_TREE_H_
#define VERIBLOCK_MERKLE_TREE_H_

#include <algorithm>
#include <deque>
#include <unordered_map>
#include <vector>
#include <veriblock/pop/arith_uint256.hpp>
#include <veriblock/pop/entities/atv.hpp>
#include <veriblock/pop/entities/merkle_path.hpp>

namespace altintegration {

//! @private
template <typename Specific, typename TxHash>
struct MerkleTree {
  using hash_t = TxHash;

  MerkleTree(Specific& instance, const std::vector<hash_t>& hashes)
      : instance(instance) {
    buildTree(hashes);
    for (int32_t i = 0, size = (int32_t)hashes.size(); i < size; i++) {
      hash_indices[hashes[i]] = i;
    }
  }

  std::vector<hash_t> getMerklePathLayers(const hash_t& hash) const {
    auto it = hash_indices.find(hash);
    VBK_ASSERT(it != hash_indices.end());
    size_t index = it->second;
    return getMerklePathLayers(index);
  }

  std::vector<hash_t> getMerklePathLayers(size_t index) const {
    VBK_ASSERT(!layers.empty());
    auto& leafs = layers[0];
    if (leafs.size() == 1) {
      // no layers
      return {};
    }
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

  const std::vector<std::vector<hash_t>>& getLayers() const { return layers; }

  const std::unordered_map<hash_t, int32_t>& getHashIndices() const {
    return hash_indices;
  }

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
      if ((n % 2) != 0u) {
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

  Specific& instance;
  std::vector<std::vector<hash_t>> layers;
  std::unordered_map<hash_t, int32_t> hash_indices;
};

//! @private
struct VbkMerkleTree {
  using hash_t = typename MerkleTree<VbkMerkleTree, uint256>::hash_t;

  enum class TreeIndex : int32_t {
    POP = 0,
    NORMAL = 1,
  };

  explicit VbkMerkleTree(const std::vector<hash_t>& normal_hashes,
                         const std::vector<hash_t>& pop_hashes)
      : pop_tree(*this, pop_hashes), normal_tree(*this, normal_hashes) {}

  hash_t hash(const hash_t& a, const hash_t& b) { return sha256(a, b); }

  std::vector<hash_t> finalizePath(std::vector<hash_t> path) {
    // opposite tree merkle root (we don't have the opposite tree)
    path.emplace_back();

    // metapackage hash
    path.emplace_back();
    return path;
  }

  hash_t finalizeRoot() {
    if (this->pop_tree.getLayers().empty() &&
        this->normal_tree.getLayers().empty()) {
      return hash_t{};
    }

    if ((this->pop_tree.getLayers().size() == 1) &&
        this->normal_tree.getLayers().empty()) {
      // the only layer
      VBK_ASSERT(this->pop_tree.getLayers()[0].size() == 1);
      return this->pop_tree.getLayers()[0][0];
    }

    if ((this->normal_tree.getLayers().size() == 1) &&
        this->pop_tree.getLayers().empty()) {
      // the only layer
      VBK_ASSERT(this->normal_tree.getLayers()[0].size() == 1);
      return this->normal_tree.getLayers()[0][0];
    }

    auto pop_hash = hash_t{};
    auto normal_hash = hash_t{};

    if (!this->pop_tree.getLayers().empty()) {
      pop_hash = this->pop_tree.getLayers().back()[0];
    }

    if (!this->normal_tree.getLayers().empty()) {
      normal_hash = this->normal_tree.getLayers().back()[0];
    }

    // add metapackage hash (also all zeroes) to the left subtree
    auto metapackageHash = hash_t();
    return hash(metapackageHash, hash(pop_hash, normal_hash));
  }

  hash_t getMerkleRoot() { return this->finalizeRoot(); }

  std::vector<hash_t> getMerklePathLayers(size_t index,
                                          TreeIndex treeIndex) const {
    switch (treeIndex) {
      case TreeIndex::POP:
        return this->pop_tree.getMerklePathLayers(index);
      case TreeIndex::NORMAL:
        return this->normal_tree.getMerklePathLayers(index);
      default:
        return {};
    }
  }

  VbkMerklePath getMerklePath(const hash_t& hash, TreeIndex treeIndex) const {
    VbkMerklePath merklePath;
    merklePath.subject = hash;
    merklePath.treeIndex = static_cast<int32_t>(treeIndex);
    switch (treeIndex) {
      case TreeIndex::POP: {
        auto it = this->pop_tree.getHashIndices().find(hash);
        VBK_ASSERT(it != this->pop_tree.getHashIndices().end());
        int32_t index = it->second;

        merklePath.index = index;
        merklePath.layers = this->pop_tree.getMerklePathLayers(index);
        break;
      }
      case TreeIndex::NORMAL: {
        auto it = this->normal_tree.getHashIndices().find(hash);
        VBK_ASSERT(it != this->normal_tree.getHashIndices().end());
        int32_t index = it->second;

        merklePath.index = index;
        merklePath.layers = this->normal_tree.getMerklePathLayers(index);
        break;
      }
      default:
        return merklePath;
    }

    return merklePath;
  }

 private:
  MerkleTree<VbkMerkleTree, uint256> pop_tree;
  MerkleTree<VbkMerkleTree, uint256> normal_tree;
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
    if (layers.empty()) {
      return hash_t{};
    }

    auto& root = layers.back();
    VBK_ASSERT(root.size() == 1);
    return root[0].reverse();
  }

  MerklePath getMerklePath(const hash_t& hash) const {
    auto it = hash_indices.find(hash);
    VBK_ASSERT(it != hash_indices.end());
    size_t index = it->second;

    MerklePath merklePath;
    merklePath.index = (int32_t)index;
    merklePath.subject = hash;
    merklePath.layers = getMerklePathLayers(index);
    return merklePath;
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
      return hash_t{};
    }

    auto& root = this->layers.back();
    VBK_ASSERT(root.size() == 1);
    return root[0].reverse();
  }
};

}  // namespace altintegration
#endif  // !VERIBLOCK_MERKLE_TREE_H_
